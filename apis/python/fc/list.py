import flatbuffers
import flatbuffers.flexbuffers
import typing
from fc.client import Client
from fc.common import raise_if, createKvMap, ResponseError
from fc.logging import logger
from typing import Any
from fc.fbs.fc.common import Ident, ListType
from fc.fbs.fc.request import (Request, RequestBody,
                               ListCreate, ListAdd, ListDelete, ListGetN, Range, Base)
from fc.fbs.fc.response import (ResponseBody, ListGetN as ListGetNRsp)


class List:
  "List API. If a response returns a fail, a ResponseError is raised."


  def __init__(self, client: Client):
    self.client = client


  async def create(self, *, name: str, type: str, failOnDuplicate=True) -> None:
    """
    Create a list with the given name. 
    
    - `type`: must be one of:  'int', 'uint'
    - `failOnDuplicate`: if `True` then command will fail if a list with this name 
                         already exists
    """
    try:      
      if type.lower() == 'int':
        listType = ListType.ListType.Int
      elif type.lower() == 'uint':
        listType = ListType.ListType.UInt
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


  async def add(self, *, name: str, items: typing.List[int], pos: int) -> None:
    await self._do_add(name, items, pos, Base.Base.Tail if pos < 0 else Base.Base.Head)


  async def add_head(self, *, name: str, items: typing.List[int]) -> None:
    await self._do_add(name, items, 0, Base.Base.Head)


  async def add_tail(self, *, name: str, items: typing.List[int]) -> None:
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


  async def delete_all(self):
    await self.delete(names=[])


  async def get_head(self, name: str):
    return await self.get_n(name, start=0, count=1)
  
  
  async def get_tail(self, name: str):
    return await self.get_n(name, start=-1, count=1)
  

  async def get_n(self, name: str, *, start: int = 0, count: int = 0):
    "If count is 0, returns all items from start"
    try:
      raise_if(len(name) == 0, 'name is empty')

      fb = flatbuffers.Builder(initialSize=128)

      nameOffset = fb.CreateString(name)

      ListGetN.Start(fb)
      ListGetN.AddName(fb, nameOffset)
      ListGetN.AddStart(fb, start)
      ListGetN.AddCount(fb, count)
      body = ListGetN.End(fb)

      self._complete_request(fb, body, RequestBody.RequestBody.ListGetN)
      rsp = await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListGetN)

      union_body = ListGetNRsp.ListGetN()
      union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)

      # this is how we get a flexbuffer from a flatbuffer
      result = flatbuffers.flexbuffers.Loads(union_body.ItemsAsNumpy().tobytes())
      return result
    except Exception as e:
      print(e)
      raise
  

  # async def get_range(self, name: str, *, start=0, stop=1):
  #   return await self._do_get(name, start, stop=stop)


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