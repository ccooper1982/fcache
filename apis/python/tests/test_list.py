import unittest
from base import ListTest
from fc.common import ResponseError


class KV(ListTest):
  async def test_create(self):
    # TODO add more tests when exists() available
    await self.list.create(name='l', type='int', failOnDuplicate=False)
    await self.list.create(name='l', type='int', failOnDuplicate=False)

    with self.assertRaises(ResponseError):
      await self.list.create(name='l', type='int', failOnDuplicate=True)


  async def test_add_head(self):
    await self.list.create(name='l', type='int')
    

