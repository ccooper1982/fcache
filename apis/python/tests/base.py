from unittest import IsolatedAsyncioTestCase
from fc.client import fcache
from fc.kv import KV
from fc.list import UnsortedList, SortedList


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


class UnsortedListTest(FcTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()
    self.list = UnsortedList(self.client)
    await self.list.delete_all()

    
class SortedListTest(FcTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()
    self.list = SortedList(self.client)
    await self.list.delete_all()

    