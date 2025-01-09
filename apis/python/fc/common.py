from fc.fbs.fc.response import Response, ResponseBody, Status
import flatbuffers
import flatbuffers.flexbuffers
import array


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
    return self(bodyType, status=status)
  
  @classmethod
  def bodyTypeError(self, bodyType: ResponseBody.ResponseBody):
    "If body type is incorrect, status is irrelevant"
    return self(bodyType)
  

def createKvMap(kv: dict) -> bytearray:
  """Creates a flexbuffer map, populated with `kv`. """
  b = flatbuffers.flexbuffers.Builder()
  b.MapFromElements(kv)
  return b.Finish()


def createIntArray(items: list[int], unsigned=False):
  # q: int8, Q: uint8
  vec = array.array('Q' if unsigned else 'q')
  vec.fromlist(items)
  return vec


def createFloatArray(items: list[float]):
  vec = array.array('f')  # TODO allow double
  vec.fromlist(items)
  return vec




def raise_if_fail(rsp: bytes, expectedRspBody: ResponseBody) -> Response.Response:
  """Confirm the response status is successful and the body type is expected.
  
  If status or body type checks fail, raise a ResponseError. Otherwise,
  return the deserialised Response object.
  """
  response = Response.Response.GetRootAs(rsp)
  if response.Status() != Status.Status.Ok:
    raise ResponseError.statusError(response.BodyType(), response.Status())
  elif response.BodyType() != expectedRspBody:
    raise ResponseError.bodyTypeError(response.BodyType())
  else:
    return response
  

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
  

