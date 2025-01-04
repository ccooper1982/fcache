from fc.common import ResponseError, raise_if_fail
from fc.connection import Connection
from fc.logging import logger
from typing import Tuple, List



class NdbClient:
  def __init__(self):
    self.uri = ''
    self.ws = Connection()
    
  # def __str__(self):
  #   if self.ws.opened != None and self.ws.opened:
  #     connected = 'Connected' 
  #   else:
  #     connected = 'Disconnected' 

  #   return f'NDB - {self.uri} - {connected}'


  async def open(self, uri: str) -> None:
    if uri == '':
      raise ValueError('URI empty')
        
    await self.ws.start(uri)
    logger.debug('Client connected')
    
    
  async def close(self):
    await self.ws.close()
 

  async def sendCmd(self, data:bytearray) -> bytes:
    rsp = await self.ws.query2(data)
    raise_if_fail(rsp)
    return rsp
  
