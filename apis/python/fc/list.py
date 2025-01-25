import flatbuffers
import flatbuffers.flexbuffers
import typing
from fc.client import Client
from fc.common import raise_if
from fc.logging import logger
from fc.fbs.fc.common import Ident, ListType
from fc.fbs.fc.request import (Request, RequestBody,
                               ListCreate, ListAdd, ListDelete, ListGetN, ListGetRange, Range, Base)
from fc.fbs.fc.response import (ResponseBody, ListGetN as ListGetNRsp, ListGetRange as ListGetRangeRsp)


class List:
  "List API. If a request fails a ResponseError is raised."


  def __init__(self, client: Client):
    self.client = client


  async def create(self, name: str, *, type: str, failOnDuplicate=True) -> None:
    """
    Create a list with the given name. 
    
    - `type`: must be one of:  'int', 'uint'
    - `failOnDuplicate`: if `True` then command will fail if a list with this name 
                         already exists
    """
    try:      
      if type.lower() == 'int':
        listType = ListType.ListType.Int
      # elif type.lower() == 'uint':
      #   listType = ListType.ListType.UInt
      elif type.lower() == 'float':
        listType = ListType.ListType.Float
      elif type.lower() == 'str':
        listType = ListType.ListType.String
      else:
        raise ValueError('listType invalid')

      fb = flatbuffers.Builder(initialSize=1024)
      
      nameOffset = fb.CreateString(name)

      ListCreate.Start(fb)
      ListCreate.AddName(fb, nameOffset)
      ListCreate.AddType(fb, listType)
      body = ListCreate.End(fb)

      self._complete_request(fb, body, RequestBody.RequestBody.ListCreate)

      await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListCreate, allowDuplicate=not failOnDuplicate)
    except Exception as e:
      logger.error(e)
      raise


  async def add(self, name: str, items: typing.List[int], *, pos: int) -> None:
    await self._do_add(name, items, pos, Base.Base.Tail if pos < 0 else Base.Base.Head)


  async def add_head(self, name: str, items: typing.List[int]) -> None:
    await self._do_add(name, items, 0, Base.Base.Head)


  async def add_tail(self, name: str, items: typing.List[int]) -> None:
    await self._do_add(name, items, 0, Base.Base.Tail)


  async def delete(self, *, names: typing.List[str]) -> None:
    fb = flatbuffers.Builder(initialSize=1024)
    
    nameOffsets = []
    for name in names:
      nameOffsets.append(fb.CreateString(name))

    ListDelete.StartNameVector(fb, len(names))
    for offset in nameOffsets:
      fb.PrependUOffsetTRelative(offset)
    namesVec = fb.EndVector()

    ListDelete.Start(fb)
    ListDelete.AddName(fb, namesVec)
    body = ListDelete.End(fb)

    self._complete_request(fb, body, RequestBody.RequestBody.ListDelete)
    await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListDelete)


  async def delete_all(self) -> None:
    await self.delete(names=[])


  async def get_head(self, name: str):
    items = await self.get_n(name, start=0, count=1)
    return items[0] if len(items) > 0 else None
  
  
  async def get_tail(self, name: str):
    items  = await self.get_n_reverse(name, start=0, count=1)
    return items[0] if len(items) > 0 else None
  

  async def get_n(self, name: str, *, start: int = 0, count: int = 0) -> list:
    """
    Get items, beginning at `start` for `count` items. 
    `count` being `0` means get remaining items from `start` to end"""
    return await self._do_get_n(name, True, start, count)
  

  async def get_n_reverse(self, name: str, *, start: int = 0, count: int = 0) -> list:
    return await self._do_get_n(name, False, start, count)
  

  async def get_range(self, name: str, *, start:int, stop: int = None) -> list:
    if stop is not None:
      if start == stop:
        return []
      if (start < 0 and stop < 0) or (start > 0 and stop > 0):
        raise_if(start > stop, 'invalid range')

    return await self._do_get_range(name, Base.Base.Head, start, stop)


  async def get_range_reverse(self, name: str, *, start:int, stop: int = None) -> list:
    if stop is not None:
      if start == stop:
        return []
      
    return await self._do_get_range(name, Base.Base.Tail, start, stop)


  async def _do_get_range(self, name: str, base: Base.Base, start:int, stop: int = None) -> list:    
    try:
      raise_if(len(name) == 0, 'name is empty')

      fb = flatbuffers.Builder(initialSize=128)

      nameOffset = fb.CreateString(name)

      Range.Start(fb)
      Range.AddStart(fb, start)
      Range.AddStop(fb, stop if stop is not None else 0)
      Range.AddHasStop(fb, stop is not None)
      rangeOffset = Range.End(fb)

      ListGetRange.Start(fb)
      ListGetRange.AddName(fb, nameOffset)
      ListGetRange.AddRange(fb, rangeOffset)
      ListGetRange.AddBase(fb, base)
      body = ListGetRange.End(fb)

      self._complete_request(fb, body, RequestBody.RequestBody.ListGetRange)
      rsp = await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListGetRange)

      union_body = ListGetRangeRsp.ListGetRange()
      union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)
      result = flatbuffers.flexbuffers.Loads(union_body.ItemsAsNumpy().tobytes())
      return result
    except Exception as e:
      logger.error(e)
      print(e)
      raise


  async def _do_get_n(self, name: str, forwards: bool, start: int, count: int):
    try:
      raise_if(len(name) == 0, 'name is empty')

      fb = flatbuffers.Builder(initialSize=128)

      nameOffset = fb.CreateString(name)

      ListGetN.Start(fb)
      ListGetN.AddName(fb, nameOffset)
      ListGetN.AddStart(fb, start)
      ListGetN.AddCount(fb, count)
      ListGetN.AddBase(fb, Base.Base.Head if forwards else Base.Base.Tail)
      body = ListGetN.End(fb)

      self._complete_request(fb, body, RequestBody.RequestBody.ListGetN)
      rsp = await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListGetN)

      union_body = ListGetNRsp.ListGetN()
      union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)
      result = flatbuffers.flexbuffers.Loads(union_body.ItemsAsNumpy().tobytes())
      return result
    except Exception as e:
      logger.error(e)
      raise
  
  
  async def _do_add(self, name: str, items: typing.List[int], pos: int, base: Base.Base) -> None:
    if len(items) == 0:
      raise ValueError('items cannot be empty')

    try:
      fbb = flatbuffers.flexbuffers.Builder()
      fbb.TypedVectorFromElements(items)
      itemsVector = fbb.Finish()
      

      fb = flatbuffers.Builder(initialSize=1024)
      
      nameOffset = fb.CreateString(name)
      itemOffset = fb.CreateByteVector(itemsVector)

      ListAdd.Start(fb)
      ListAdd.AddName(fb, nameOffset)
      ListAdd.AddItems(fb, itemOffset)
      ListAdd.AddPosition(fb, pos)
      ListAdd.AddBase(fb, base)
      body = ListAdd.End(fb)
      
      self._complete_request(fb, body, RequestBody.RequestBody.ListAdd)
      await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListAdd)
    except Exception as e:
      logger.error(e)
      raise


  def _complete_request(self, fb: flatbuffers.Builder, body: int, bodyType: RequestBody.RequestBody):
    try:
      Request.RequestStart(fb)
      Request.AddIdent(fb, Ident.Ident.List)
      Request.AddBodyType(fb, bodyType)
      Request.AddBody(fb, body)
      fb.Finish(Request.RequestEnd(fb))
    except Exception as e:
      print(e)
      logger.error(e)
      fb.Clear()