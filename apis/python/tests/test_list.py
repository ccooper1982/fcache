import unittest
from base import ListTest
from fc.common import ResponseError


class KV(ListTest):
  async def test_create(self):
    await self.list.create('list1', 'int')