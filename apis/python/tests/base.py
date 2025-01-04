from unittest import IsolatedAsyncioTestCase
from fc.client import FcClient
from fc.kv import KV


class FcTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = FcClient()
    
    connected = True
    try:
      await self.client.open('ws://127.0.0.1:1987')
    except:
      connected = False
    finally:
      self.assertTrue(connected)


class KvTest(FcTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()
    self.kv = KV(self.client)
    await self.kv.clear()