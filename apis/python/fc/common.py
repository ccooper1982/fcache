from fc.fbs.fc.response import Response, ResponseBody, Status
import flatbuffers.flexbuffers as FlexBuffers
import array


def statusToString(status: Status.Status):
  match(status):
    case Status.Status.Ok:
      return 'OK'
    case Status.Status.Fail:
      return 'Fail'
    case Status.Status.ParseError:
      return 'Parse Error'
    case Status.Status.CommandUnknown:
      return 'Command unknown'
    case Status.Status.Duplicate:
      return 'Duplicate'


class FcException(Exception):
  def __init__(self, msg):
    super().__init__(msg)


class ResponseError(FcException):
  def __init__(self, *, bodyType=None, fbStatus=None, connectionFail=False):
    if connectionFail:
      super().__init__(f'Disconnected')
    elif fbStatus != Status.Status.Ok:
      super().__init__(f'Response Error: Status: {statusToString(fbStatus)}')
    else:
      super().__init__(f'Response Error: Unexpected Body Type: {bodyType}')

    self.status = fbStatus
    self.bodyType = bodyType
    self.connectionFail = connectionFail


  @classmethod
  def statusError(self, bodyType: ResponseBody.ResponseBody, status: Status.Status):
    return self(bodyType=bodyType, fbStatus=status)
  
  @classmethod
  def bodyTypeError(self, bodyType: ResponseBody.ResponseBody):
    "If body type is incorrect, status is irrelevant"
    return self(bodyType=bodyType)
  
  @classmethod
  def disconnected(self):
    "If connection failed"
    return self(connectionFail=True)
  

def createKvMap(kv: dict) -> bytearray:
  """Creates a flexbuffer map, populated with `kv`. """
  # a hijacked flatbuffers.flexbuffers.MapFromElements(),
  # with checks that a list value:
  #   - is not empty
  #   - all elements are the same type
  #   - all are serialised as TypedVector
  #   - list of strings is serialised as FBT_VECTOR_KEY
  fb = FlexBuffers.Builder()
  
  with fb.Map():
    try:
      # value is None?
      for key, value in kv.items():
        if not isinstance(key, str):
          raise ValueError('Key must be a string')
        
        fb.Key(key)

        if isinstance(value, bool):
          fb.Bool(value)
        elif isinstance(value, int):
          fb.Int(value)
        elif isinstance(value, float):
          fb.Float(value)
        elif isinstance(value, str):
          fb.String(value)
        elif isinstance(value, bytes):
          fb.Blob(value)
        elif isinstance(value, array.array):
          fb.TypedVectorFromElements(value)
        elif isinstance(value, list):
          _createTypedVector(fb, key, value)
        else:          
          raise ValueError(f'Key {key}: has invalid value type')
    except:
      # we likely added a key without corresponding value, and in either case,
      # the buffer state is unknown if we're here, so clear
      fb.Clear()
      raise
  return fb.Finish()


def _createTypedVector(fb: FlexBuffers.Builder, key: str, items: list):
  # cannot allow empty lists because at least one item is required
  # to know the type of the TypedVector.
  # TODO Should probably create a workaround for this.
  if len(items) == 0:
    raise ValueError(f'Key {key}: contains empty list')
  
  # all elements must be same type
  elementType = type(items[0])
  if not all(isinstance(s, elementType) for s in items):
    raise ValueError(f'Key {key}: all elements must be the same type')
  
  if elementType == str:
    fb.TypedVectorFromElements(items, element_type=FlexBuffers.Type.KEY)
  else:
    fb.TypedVectorFromElements(items)
  

# def createIntArray(items: list[int], unsigned=False):
#   # q: int8, Q: uint8
#   vec = array.array('Q' if unsigned else 'q')
#   vec.fromlist(items)
#   return vec


# def createFloatArray(items: list[float]):
#   vec = array.array('f')  # TODO allow double
#   vec.fromlist(items)
#   return vec


def _raise_if_fail(rsp: bytes, expectedRspBody: ResponseBody, status=(Status.Status.Ok)) -> Response.Response:
  """Confirm the response status is successful and the body type is expected.
  
  If status or body type checks fail, raise a ResponseError. Otherwise,
  return the deserialised Response object.
  """
  if rsp:
    response = Response.Response.GetRootAs(rsp)
    if response.Status() not in status:
      raise ResponseError.statusError(response.BodyType(), response.Status())
    elif response.BodyType() != expectedRspBody:      
      raise ResponseError.bodyTypeError(bodyType=response.BodyType())
    else:
      return response
  else:
    raise ResponseError.disconnected()
  

def raise_if_fail(rsp: bytes, expectedRspBody: ResponseBody, allowDuplicate=False) -> Response.Response:
  """Confirm the response status is successful and the body type is expected.
  
  If status or body type checks fail, raise a ResponseError. Otherwise,
  return the deserialised Response object.
  """
  if allowDuplicate:
    return _raise_if_fail(rsp, expectedRspBody, status=(Status.Status.Ok, Status.Status.Duplicate))
  else:
    return _raise_if_fail(rsp, expectedRspBody, status=(Status.Status.Ok,))
  

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
  if not condition:
    raise errorType(msg)
  

