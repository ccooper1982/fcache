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

    await self.list.add_head('l', items=[5])
    out = await self.list.get_n('l')
    self.assertListEqual(out, [5])

    await self.list.add_head('l', items=[4])
    out = await self.list.get_n('l')
    self.assertListEqual(out, [4,5])

    await self.list.add_head('l', items=[1,2,3])
    out = await self.list.get_n('l')
    self.assertListEqual(out, [1,2,3,4,5])


  async def test_add_tail(self):
    await self.list.create(name='l', type='int')

    await self.list.add_tail('l', items=[5])
    out = await self.list.get_n('l', start=0)
    self.assertListEqual(out, [5])

    await self.list.add_tail('l', items=[6,7,8])
    out = await self.list.get_n('l')
    self.assertListEqual(out, [5,6,7,8])


  async def test_add(self):
    await self.list.create(name='l', type='int')

    # same as add_head
    await self.list.add('l', items=[1,2,3], pos=0)
    out = await self.list.get_n('l')
    self.assertListEqual(out, [1,2,3])

    # same as add_tail
    await self.list.add('l', items=[7,8,9], pos=3)
    out = await self.list.get_n('l')
    self.assertListEqual(out, [1,2,3,7,8,9])

    # insert mid
    await self.list.add('l', items=[4,5,6], pos=3)
    out = await self.list.get_n('l')
    self.assertListEqual(out, [1,2,3,4,5,6,7,8,9])

    # as add_head() but with items already in list
    await self.list.add('l', items=[-1,0], pos=0)
    out = await self.list.get_n('l')
    self.assertListEqual(out, [-1,0,1,2,3,4,5,6,7,8,9])


  async def test_get_n(self):
    await self.list.create(name='l', type='int') 
    await self.list.add_head('l', items=[1,2,3,4,5,6,7,8,9,10])

    # everything from head
    self.assertListEqual(await self.list.get_n('l',start=0), [1,2,3,4,5,6,7,8,9,10])
    # everything from mid
    self.assertListEqual(await self.list.get_n('l',start=5), [6,7,8,9,10])
    # everything from tail
    self.assertListEqual(await self.list.get_n('l',start=9), [10])
    # define count
    self.assertListEqual(await self.list.get_n('l',start=4, count=2), [5,6])
    # count out of range
    self.assertListEqual(await self.list.get_n('l',start=4, count=20), [5,6,7,8,9,10])


  async def test_get_n_reverse(self):
    await self.list.create(name='l', type='int') 
    await self.list.add_head('l', items=[1,2,3,4,5,6,7,8,9,10])

    self.assertListEqual(await self.list.get_n_reverse('l',start=0), [10,9,8,7,6,5,4,3,2,1])
    self.assertListEqual(await self.list.get_n_reverse('l',start=5), [5,4,3,2,1])
    self.assertListEqual(await self.list.get_n_reverse('l',start=9), [1])
    self.assertListEqual(await self.list.get_n_reverse('l',start=7, count=2), [3,2])
    self.assertListEqual(await self.list.get_n_reverse('l',start=7, count=20), [3,2,1])

  
  async def test_get_range_forward(self):
    await self.list.create(name='l', type='int') 
    await self.list.add_head('l', items=[1,2,3,4,5,6,7,8,9,10])

    # everything from head
    self.assertListEqual (await self.list.get_range('l',start=0), [1,2,3,4,5,6,7,8,9,10])
    # everything from mid
    self.assertListEqual (await self.list.get_range('l',start=7), [8,9,10])
    # everything from tail
    self.assertListEqual (await self.list.get_range('l',start=9), [10])
    
    # positive positions
    self.assertListEqual (await self.list.get_range('l',start=3, stop=5), [4,5])
    # stop negative
    self.assertListEqual (await self.list.get_range('l',start=3, stop=-2), [4,5,6,7,8])
    # start negative
    self.assertListEqual (await self.list.get_range('l',start=-8, stop=4), [3,4])
    # both negative
    self.assertListEqual (await self.list.get_range('l',start=-8, stop=-5), [3,4,5])


  async def test_get_range_reverse(self):
    await self.list.create(name='l', type='int') 
    await self.list.add_head('l', items=[1,2,3,4,5,6,7,8,9,10])

    # everything from tail
    self.assertListEqual (await self.list.get_range('l',start=-1, stop=0), [10,9,8,7,6,5,4,3,2])