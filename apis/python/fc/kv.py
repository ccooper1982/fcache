from fc.client import NdbClient
from fc.common import raise_if, CreateKvArray
from fc.logging import logger
from typing import List
import flatbuffers
import flatbuffers.flexbuffers
from fc.fbs.fc.request import (Request, RequestBody,
                               KVSet,
                               KVGet,
                               KVRmv,                               
                               KVAdd,
                               KVCount,
                               KVContains)

from fc.fbs.fc.response import (Response, ResponseBody, Status,
                                KVGet as KVGetRsp,
                                KVRmv as KVRmvRsp,
                                KVCount as KVCountRsp,
                                KVContains as KVContainsRsp)

class KV:
  "Key Value"


  def __init__(self, client: NdbClient):
    self.client = client


  async def set(self, kv: dict) -> None:
    await self._doSetAdd(kv, RequestBody.RequestBody.KVSet)


  async def add(self, kv: dict) -> None:
    await self._doSetAdd(kv, RequestBody.RequestBody.KVAdd)
  

  async def get(self, key=None, keys=[]) -> dict:
    raise_if(key is None and len(keys) == 0, 'key or keys must be set')

    if len(keys) == 0:
      keys = [key]

    try:
      fb = flatbuffers.Builder()
      keysOff = self._createStrings(fb, keys)

      KVGet.Start(fb)
      KVGet.AddKeys(fb, keysOff)
      body = KVGet.End(fb)

      self._completeRequest(fb, body, RequestBody.RequestBody.KVGet)
      
      rspBuffer = await self.client.sendCmd2(fb.Output())

      rsp = Response.Response.GetRootAs(rspBuffer)
      if rsp.BodyType() == ResponseBody.ResponseBody.KVGet:
        union_body = KVGetRsp.KVGet()
        union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)
        # this is how we get a flexbuffer from a flatbuffer
        return flatbuffers.flexbuffers.Loads(union_body.KvAsNumpy().tobytes())

    except Exception as e:
      logger.error(e)


  async def remove(self, key=None, keys=[]) -> None:
    raise_if(key is None and len(keys) == 0, 'key or keys must be set')

    if len(keys) == 0:
      keys = [key]

    try:
      fb = flatbuffers.Builder()
      keysOff = self._createStrings(fb, keys)

      KVRmv.Start(fb)
      KVRmv.AddKeys(fb, keysOff)
      body = KVRmv.End(fb)

      self._completeRequest(fb, body, RequestBody.RequestBody.KVRmv)

      await self.client.sendCmd2(fb.Output())

    except Exception as e:
      logger.error(e)


  async def count(self) -> int:
    fb = flatbuffers.Builder()
    KVCount.Start(fb)    
    body = KVCount.End(fb)
    self._completeRequest(fb, body, RequestBody.RequestBody.KVCount)

    rspBuffer = await self.client.sendCmd2(fb.Output())

    rsp = Response.Response.GetRootAs(rspBuffer)
    if rsp.BodyType() == ResponseBody.ResponseBody.KVCount:
      union_body = KVCountRsp.KVCount()
      union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)
      return union_body.Count()


  async def contains(self, keys=[]) -> list:
    raise_if(len(keys) == 0, 'keys is empty')

    try:
      fb = flatbuffers.Builder()
      keysOff = self._createStrings(fb, keys)

      KVContains.Start(fb)
      KVContains.AddKeys(fb, keysOff)
      body = KVContains.End(fb)

      self._completeRequest(fb, body, RequestBody.RequestBody.KVContains)
      
      rspBuffer = await self.client.sendCmd2(fb.Output())

      rsp = Response.Response.GetRootAs(rspBuffer)
      if rsp.BodyType() == ResponseBody.ResponseBody.KVContains:
        union_body = KVContainsRsp.KVContains()
        union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)
        
        # The API does not return all strings in an iterable, you have to request
        # each item by index. And each is returned as bytes rather than str
        exist = []
        for i in range(union_body.KeysLength()):
          exist.append(union_body.Keys(i).decode('utf-8'))
        return exist
    except Exception as e:
      logger.error(e)



  ## Helpers ##
  def _createStrings (self, fb: flatbuffers.Builder, strings: list) -> int:
    keysOffsets = []
    for key in strings:
      keysOffsets.append(fb.CreateString(key))
    
    fb.StartVector(4, len(strings), 4)
    for off in keysOffsets:
        fb.PrependUOffsetTRelative(off)
    return fb.EndVector()
    

  def _completeRequest(self, fb: flatbuffers.Builder, body: int, bodyType: RequestBody.RequestBody):
    try:
      Request.RequestStart(fb)
      Request.AddBodyType(fb, bodyType)
      Request.AddBody(fb, body)
      req = Request.RequestEnd(fb)

      fb.Finish(req)
    except Exception as e:
      logger.error(e)
      fb.Clear()


  async def _doSetAdd(self, kv: dict, requestType: RequestBody.RequestBody) -> None:
    raise_if(len(kv) == 0, 'keys empty')

    try:
      fb = flatbuffers.Builder()
      kvVec = fb.CreateByteVector(CreateKvArray(kv))

      if requestType is RequestBody.RequestBody.KVSet:
        KVSet.Start(fb)
        KVSet.AddKv(fb, kvVec)
        body = KVSet.End(fb)
      else:
        KVAdd.Start(fb)
        KVAdd.AddKv(fb, kvVec)
        body = KVSet.End(fb)

      self._completeRequest(fb, body, requestType)

      await self.client.sendCmd2(fb.Output())
    except Exception as e:
      logger.error(e)



class KV_Old:
  "Key Value"

  async def count(self) -> int:
    rsp = await self.client.sendCmd(self.cmds.COUNT_REQ, self.cmds.COUNT_RSP, {})
    return rsp[self.cmds.COUNT_RSP]['cnt']


  async def contains(self, keys: tuple) -> List[str]:
    rsp = await self.client.sendCmd(self.cmds.CONTAINS_REQ, self.cmds.CONTAINS_RSP, {'keys':keys})
    return rsp[self.cmds.CONTAINS_RSP]['contains']

  
  async def keys(self) -> List[str]:
    rsp = await self.client.sendCmd(self.cmds.KEYS_REQ, self.cmds.KEYS_RSP, {})
    return rsp[self.cmds.KEYS_RSP]['keys']
  

  async def clear(self) -> int:
    rsp = await self.client.sendCmd(self.cmds.CLEAR_REQ, self.cmds.CLEAR_RSP, {})
    return rsp[self.cmds.CLEAR_RSP]['cnt']
        

  async def clear_set(self, keys: dict) -> int:
    rsp = await self.client.sendCmd(self.cmds.CLEAR_SET_REQ, self.cmds.CLEAR_SET_RSP, {'keys':keys})
    return rsp[self.cmds.CLEAR_SET_RSP]['cnt'] 

