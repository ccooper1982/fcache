import flatbuffers
import flatbuffers.flexbuffers
from fc.client import Client
from fc.common import raise_if, createKvMap, ResponseError
from fc.logging import logger
from typing import Any
from fc.fbs.fc.common import Ident
from fc.fbs.fc.request import (Request, RequestBody,
                               KVSet,
                               KVGet,
                               KVRmv,                               
                               KVAdd,
                               KVCount,
                               KVContains,
                               KVClear,
                               KVClearSet)
from fc.fbs.fc.response import (KVGet as KVGetRsp,
                                KVCount as KVCountRsp,
                                KVContains as KVContainsRsp)


class KV:
  "Key Value API. If a response returns a fail, a ResponseError is raised."


  def __init__(self, client: Client):
    self.client = client


  async def set(self, kv: dict) -> None:
    await self._do_set_add(kv, RequestBody.RequestBody.KVSet)


  async def add(self, kv: dict) -> None:
    await self._do_set_add(kv, RequestBody.RequestBody.KVAdd)
  

  async def get(self, *, key=None, keys=[]) -> dict | Any:
    """Get a single key or multiple keys.
    
    @param: key To get a single key. Only the value is returned.
    @param: keys To get multiple keys. All keys/vals returned in a dict.

    """
    raise_if(key is None and len(keys) == 0, 'key or keys must be set')

    isSingleKey = len(keys) == 0
    if isSingleKey:
      keys = [key]

    try:
      fb = flatbuffers.Builder(initialSize=1024)
      keysOff = self._create_strings(fb, keys)

      KVGet.Start(fb)
      KVGet.AddKeys(fb, keysOff)
      body = KVGet.End(fb)

      self._complete_request(fb, body, RequestBody.RequestBody.KVGet)
      
      rsp = await self.client.sendCmd(fb.Output(), RequestBody.RequestBody.KVGet)

      union_body = KVGetRsp.KVGet()
      union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)

      # this is how we get a flexbuffer from a flatbuffer
      result = flatbuffers.flexbuffers.Loads(union_body.KvAsNumpy().tobytes())
      if isSingleKey:
        return result[key] if key in result else None  # single key, so return value
      else:
        return result # multiple keys, return dict
    except Exception as e:
      print(e)
      logger.error(e)


  async def remove(self, *, key='', keys=[]) -> None:
    raise_if(len(key) == 0 and len(keys) == 0, 'key or keys must be set')

    if len(keys) == 0:
      keys = [key]

    try:
      fb = flatbuffers.Builder(initialSize=1024)
      keysOff = self._create_strings(fb, keys)

      KVRmv.Start(fb)
      KVRmv.AddKeys(fb, keysOff)
      body = KVRmv.End(fb)

      self._complete_request(fb, body, RequestBody.RequestBody.KVRmv)

      await self.client.sendCmd(fb.Output(), RequestBody.RequestBody.KVRmv)
    except Exception as e:
      logger.error(e)


  async def count(self) -> int:
    fb = flatbuffers.Builder(initialSize=1024)
    KVCount.Start(fb)    
    body = KVCount.End(fb)
    self._complete_request(fb, body, RequestBody.RequestBody.KVCount)

    rsp = await self.client.sendCmd(fb.Output(), RequestBody.RequestBody.KVCount)
    union_body = KVCountRsp.KVCount()
    union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)
    return union_body.Count()


  async def contains(self, keys: list) -> set:
    raise_if(len(keys) == 0, 'keys is empty')

    try:
      fb = flatbuffers.Builder(initialSize=1024)
      keysOff = self._create_strings(fb, keys)

      KVContains.Start(fb)
      KVContains.AddKeys(fb, keysOff)
      body = KVContains.End(fb)

      self._complete_request(fb, body, RequestBody.RequestBody.KVContains)
      
      rsp = await self.client.sendCmd(fb.Output(), RequestBody.RequestBody.KVContains)
      union_body = KVContainsRsp.KVContains()
      union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)
      
      result = flatbuffers.flexbuffers.Loads(union_body.KeysAsNumpy().tobytes())
      return result
    except Exception as e:
      logger.error(e)


  async def clear(self) -> None:
    fb = flatbuffers.Builder(initialSize=1024)
    KVClear.Start(fb)    
    body = KVClear.End(fb)
    self._complete_request(fb, body, RequestBody.RequestBody.KVClear)

    await self.client.sendCmd(fb.Output(), RequestBody.RequestBody.KVClear)

  
  async def clear_set(self, kv:dict):
    await self._do_set_add(kv, RequestBody.RequestBody.KVClearSet)
    

  ## Helpers ##
  def _create_strings (self, fb: flatbuffers.Builder, strings: list) -> int:
    keysOffsets = []
    for key in strings:
      keysOffsets.append(fb.CreateString(key))
    
    fb.StartVector(4, len(strings), 4)
    for off in keysOffsets:
        fb.PrependUOffsetTRelative(off)
    return fb.EndVector()
    

  def _complete_request(self, fb: flatbuffers.Builder, body: int, bodyType: RequestBody.RequestBody):
    try:
      Request.RequestStart(fb)
      Request.AddIdent(fb, Ident.Ident.KV)
      Request.AddBodyType(fb, bodyType)
      Request.AddBody(fb, body)
      req = Request.RequestEnd(fb)

      fb.Finish(req)
    except Exception as e:
      logger.error(e)
      fb.Clear()


  async def _do_set_add(self, kv: dict, requestType: RequestBody.RequestBody) -> None:
    """KVSet, KVAdd and KVClearSet all use a flexbuffer map, so they all 
    use this function to populate the map from `kv`"""

    raise_if(len(kv) == 0, 'keys empty')

    try:
      fb = flatbuffers.Builder(initialSize=1024)
      kvVec = fb.CreateByteVector(createKvMap(kv))

      if requestType is RequestBody.RequestBody.KVSet:
        KVSet.Start(fb)
        KVSet.AddKv(fb, kvVec)
        body = KVSet.End(fb)
      elif requestType is RequestBody.RequestBody.KVAdd:
        KVAdd.Start(fb)
        KVAdd.AddKv(fb, kvVec)
        body = KVAdd.End(fb)
      elif requestType is RequestBody.RequestBody.KVClearSet:
        KVClearSet.Start(fb)
        KVClearSet.AddKv(fb, kvVec)
        body = KVClearSet.End(fb)
      else:
        raise ValueError("RequestBody not permitted")

      self._complete_request(fb, body, requestType)

      await self.client.sendCmd(fb.Output(), requestType)
    except ResponseError as re:
      raise
    except Exception as e:
      logger.error(e)
      raise



