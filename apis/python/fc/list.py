import flatbuffers
import flatbuffers.flexbuffers
import typing
from abc import ABC
from fc.client import Client
from fc.common import raise_if, raise_if_not
from fc.logging import logger
from fc.fbs.fc.common import Ident, ListType
from fc.fbs.fc.request import (Request, RequestBody,
                               ListCreate, ListAdd, ListDelete, ListGetRange, ListRemove, ListRemoveIf, ListIntersect, Range, Base)
from fc.fbs.fc.request import (IntValue, StringValue, FloatValue, Value)
from fc.fbs.fc.response import (ResponseBody, ListGetRange as ListGetRangeRsp, ListIntersect as ListIntersectRsp)


class List(ABC):
  "List API. If a request fails a ResponseError is raised."


  def __init__(self, client: Client):
    self.client = client


  async def _create(self, name: str, type: str, *, is_sorted:bool, fail_on_duplicate:bool) -> None:
    try:      
      if type.lower() == 'int':
        listType = ListType.ListType.Int
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
      ListCreate.AddSorted(fb, is_sorted)
      body = ListCreate.End(fb)

      self._complete_request(fb, body, RequestBody.RequestBody.ListCreate)

      await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListCreate, allowDuplicate=not fail_on_duplicate)
    except Exception as e:
      logger.error(e)
      raise

  
  async def delete(self, names: typing.List[str]) -> None:
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
  

  async def get_n(self, name: str, *, start: int = 0, count: int = None) -> list:
    """
    Get items, beginning at `start` for `count` items. 

    `count` being `None` or `0` means get all remaining items
    """
    count = None if count is None or count == 0 else start+count
    return await self._do_get_range(name, Base.Base.Head, start, count)
  

  async def get_n_reverse(self, name: str, *, start: int = 0, count: int = None) -> list:
    count = None if count is None or count == 0 else start+count
    return await self._do_get_range(name, Base.Base.Tail, start, count)
  

  async def get_range(self, name: str, *, start:int, stop: int = None) -> list:
    if not self._is_range_valid(start, stop):
      return []
    return await self._do_get_range(name, Base.Base.Head, start, stop)


  async def get_range_reverse(self, name: str, *, start:int, stop: int = None) -> list:
    if not self._is_range_valid(start, stop):
      return []
    return await self._do_get_range(name, Base.Base.Tail, start, stop)


  async def remove(self, name:str, *, start: int = 0, stop: int = None) -> None:
    raise_if_not(self._is_range_valid(start, stop), 'range invalid')
    
    fb = flatbuffers.Builder(128)

    nameOffset = fb.CreateString(name)

    Range.Start(fb)
    Range.AddStart(fb, start)
    Range.AddStop(fb, stop if stop is not None else 0)
    Range.AddHasStop(fb, stop is not None)
    rangeOffset = Range.End(fb)

    ListRemove.Start(fb)
    ListRemove.AddName(fb, nameOffset)
    ListRemove.AddRange(fb, rangeOffset)
    ListRemove.AddBase(fb, Base.Base.Head)
    body = ListRemove.End(fb)

    self._complete_request(fb, body, RequestBody.RequestBody.ListRemove)
    await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListRemove)
  

  async def remove_if_eq(self, name:str, *, start: int = 0, stop: int = None, val):
    """ Remove if nodes in range [start,stop) equals val """
    
    raise_if_not(self._is_range_valid(start, stop), 'range invalid')
    raise_if(val is None, 'val cannot be none')

    fb = flatbuffers.Builder(256)

    nameOffset = fb.CreateString(name)

    if isinstance(val, str):
      stringValOffset = fb.CreateString(val)


    if isinstance(val, int):
      IntValue.Start(fb)
      IntValue.AddV(fb, val)
      valueOffset = IntValue.End(fb)
      type = Value.Value.IntValue
    elif isinstance(val, str):
      StringValue.Start(fb)
      StringValue.AddV(fb, stringValOffset)
      valueOffset = StringValue.End(fb)
      type = Value.Value.StringValue
    elif isinstance(val, float):
      FloatValue.Start(fb)
      FloatValue.AddV(fb, val)
      valueOffset = FloatValue.End(fb)
      type = Value.Value.FloatValue
    else:
      raise ValueError('val not supported type')
        

    Range.Start(fb)
    Range.AddStart(fb, start)
    Range.AddStop(fb, stop if stop is not None else 0)
    Range.AddHasStop(fb, stop is not None)
    rangeOffset = Range.End(fb)

    ListRemoveIf.Start(fb)
    ListRemoveIf.AddName(fb, nameOffset)
    ListRemoveIf.AddRange(fb, rangeOffset)
    ListRemoveIf.AddValueType(fb, type)
    ListRemoveIf.AddValue(fb, valueOffset)
    body = ListRemove.End(fb)

    self._complete_request(fb, body, RequestBody.RequestBody.ListRemoveIf)
    await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListRemoveIf)


  ### helpers
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

  
  async def _do_add(self, name: str, items: typing.List[int|str|float], pos: int, base: Base.Base, items_sorted:bool) -> None:
    raise_if(len(items) == 0, 'items cannot be empty')
    raise_if(pos < 0, 'pos negative')

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
      ListAdd.AddItemsSorted(fb, items_sorted)
      body = ListAdd.End(fb)
      
      self._complete_request(fb, body, RequestBody.RequestBody.ListAdd)
      await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListAdd)
    except Exception as e:
      logger.error(e)
      raise


  def _is_range_valid(self, start:int, stop:int):
    # easy checks we can do without knowing the list's size
    if stop is None:
      return True    
    elif start == stop:
      return False
    elif (start < 0 and stop < 0) or (start > 0 and stop > 0):
      if start > stop:
        return False
    return True


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


## Concrete classes

class UnsortedList(List):
  def __init__(self, client):
    super().__init__(client)


  async def create(self, name: str, *, type: str, fail_on_duplicate:bool=True) -> None:
    await super()._create(name, type, is_sorted=False, fail_on_duplicate=fail_on_duplicate)


  async def add(self, name: str, items: typing.List[int|str|float], *, pos: int) -> None:
    await self._do_add(name, items, pos, Base.Base.Tail if pos < 0 else Base.Base.Head, False)


  async def add_head(self, name: str, items: typing.List[int|str|float]) -> None:
    await self._do_add(name, items, 0, Base.Base.Head, False)


  async def add_tail(self, name: str, items: typing.List[int|str|float]) -> None:
    await self._do_add(name, items, 0, Base.Base.Tail, False)



class SortedList(List):
  def __init__(self, client):
    super().__init__(client)


  async def create(self, name: str, *, type: str, fail_on_duplicate:bool = True) -> None:
    await super()._create(name, type, is_sorted=True, fail_on_duplicate=fail_on_duplicate)


  async def add(self, name: str, items: typing.List[int|str|float], items_sorted:bool = False) -> None:
    await self._do_add(name, items, 0, Base.Base.Head, items_sorted)  # pos and Base irrelevant for sorted list


  async def intersect(self, list1: str, list2: str, *,
                            l1_start:int=0, l1_stop:int=None,
                            l2_start:int=0, l2_stop:int=None,
                            new_list_name:str=None) -> None | typing.List[int|float|str]:
    
    raise_if(len(list1) == 0, 'name1 empty')
    raise_if(len(list2) == 0, 'name2 empty')

    fb = flatbuffers.Builder(128)

    name1_offset = fb.CreateString(list1)
    name2_offset = fb.CreateString(list2) 

    newName_offset = None
    if new_list_name and len(new_list_name) > 0:
      newName_offset = fb.CreateString(new_list_name)

    # TODO do a _create_range()
    Range.Start(fb)
    Range.AddStart(fb, l1_start)
    Range.AddStop(fb, 0 if l1_stop is None else l1_stop)
    Range.AddHasStop(fb, l1_stop is not None)
    range1Offset = Range.End(fb)

    Range.Start(fb)
    Range.AddStart(fb, l2_start)
    Range.AddStop(fb, 0 if l2_stop is None else l2_stop)
    Range.AddHasStop(fb, l2_stop is not None)
    range2Offset = Range.End(fb)

    ListIntersect.Start(fb)
    ListIntersect.AddList1Name(fb, name1_offset)
    ListIntersect.AddList2Name(fb, name2_offset)
    ListIntersect.AddList1Range(fb, range1Offset)
    ListIntersect.AddList2Range(fb, range2Offset)
    
    if newName_offset:
      ListIntersect.AddNewListName(fb, newName_offset)

    body = ListIntersect.End(fb)

    self._complete_request(fb, body, RequestBody.RequestBody.ListIntersect)
    rsp = await self.client.sendCmd(fb.Output(), ResponseBody.ResponseBody.ListIntersect)

    if newName_offset is None:    
      union_body = ListIntersectRsp.ListIntersect()
      union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)
      result = flatbuffers.flexbuffers.Loads(union_body.ItemsAsNumpy().tobytes())
      return result
