from fc.fbs.fc.response import Response, ResponseBody, Status
import flatbuffers
import flatbuffers.flexbuffers


class FcException(Exception):
  def __init__(self, msg):
    super().__init__(msg)


class ResponseError(FcException):
  def __init__(self, bodyType: ResponseBody.ResponseBody, status = None):
    if status == None:
      super().__init__(f'Response Error: Status {status}')
    else:
      super().__init__(f'Response Error: Unexpected Body Type {bodyType}')

    self.status = status
    self.bodyType = bodyType

  @classmethod
  def statusError(self, bodyType: ResponseBody.ResponseBody, status: Status.Status):
    return self(bodyType, status)
  
  @classmethod
  def bodyTypeError(self, bodyType: ResponseBody.ResponseBody):
    "If body type is incorrect, status is irrelevant"
    return self(bodyType)
  

def createKvMap(kv: dict) -> bytearray:
  """Creates a flexbuffer map, populating with `kv`. """
  b = flatbuffers.flexbuffers.Builder()
  b.MapFromElements(kv)
  return b.Finish()


def raise_if_fail(rsp: bytes, expectedRspBody: ResponseBody):
  response = Response.Response.GetRootAs(rsp)
  if response.Status() is not Status.Status.Ok:
    raise ResponseError.statusError(response.BodyType(), response.Status())
  elif response.BodyType() is not expectedRspBody:
    raise ResponseError.bodyTypeError(response.BodyType())
  

def raise_if_empty (value: str):
  if value == '':
    raise ValueError('value empty')


def raise_if_lt (value: int, maxValue:int, msg: str):
  if value < maxValue:
    raise ValueError(msg)


def raise_if_equal (val1, val2, msg:str):
  if val1 == val2:  
    raise ValueError(msg)
  

def raise_if (condition, msg:str, errorType = ValueError):
  if condition:
    raise errorType(msg)


def raise_if_not (condition, msg:str, errorType = ValueError):
  if not raise_if(condition, msg, errorType):
    raise errorType(msg)
  

