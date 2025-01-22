from unittest import IsolatedAsyncioTestCase
from fc.client import Client, fcache
from fc.kv import KV
from fc.list import List


class FcTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    connected = True
    try:
      self.client = await fcache('ws://127.0.0.1:1987')
    except:
      connected = False
    finally:
      self.assertTrue(connected)


class KvTest(FcTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()
    self.kv = KV(self.client)
    await self.kv.clear()


class ListTest(FcTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()
    self.list = List(self.client)
    #await self.list.clear() TODO