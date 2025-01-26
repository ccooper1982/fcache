from fc.common import raise_if_fail, raise_if
from fc.connection import Connection
from fc.logging import logger
from fc.fbs.fc.response import ResponseBody, Response



class Client:
  def __init__(self):
    self.uri = ''
    self.ws = Connection()


  async def open(self, uri: str) -> None:
    if uri == '':
      raise ValueError('URI empty')
        
    await self.ws.start(uri)
    logger.debug('Client connected')
    
    
  async def close(self):
    await self.ws.close()
 

  async def sendCmd(self, data:bytearray, expectedRspBody: ResponseBody, allowDuplicate=False) -> Response.Response:
    rsp = await self.ws.query(data)
    return raise_if_fail(rsp, expectedRspBody, allowDuplicate)
    
  
  
async def fcache(uri='', ip='', port=1987) -> Client:
  """
  Opens a websocket connection to the server, returning a Client object,
  or raises an OSError if connect fails.

  Set either the full URI, or `ip` and `port` separately.

  If `uri` is set, the ip and port params are ignored.

  NOTE: only 'ws' is supported, not 'wss'.
  """
  client = Client()

  raise_if(uri == '' and ip == '', 'uri or ip must be set')
  raise_if(len(uri) and len(ip), 'uri and ip are set')

  if uri == '':
    uri = f'ws://{ip}:{port}'

  await client.open(uri)
  return client

