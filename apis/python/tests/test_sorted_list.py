from base import SortedListTest
from fc.common import ResponseError


class SortedList(SortedListTest):
  """
  This doesn't test all commands, such as get_range() and reverse version.
  These are the same irrespective of sorted/unsorted.
  """
  async def test_create(self):
    await self.list.create('l', type='int', fail_on_duplicate=False)
    await self.list.create('l', type='int', fail_on_duplicate=False)

    with self.assertRaises(ResponseError):
      await self.list.create('l', type='int', fail_on_duplicate=True)


  async def test_add(self):
    await self.list.create('l', type='int')

    size = await self.list.add('l', [1,2,3])
    self.assertEqual(size, 3)
    self.assertListEqual(await self.list.get_n('l'), [1,2,3])

    size = await self.list.add('l', [8,7,9])
    self.assertEqual(size, 6)
    self.assertListEqual(await self.list.get_n('l'), [1,2,3,7,8,9])

    size = await self.list.add('l', [4,5,6], items_sorted=True)
    self.assertEqual(size, 9)
    self.assertListEqual(await self.list.get_n('l'), [1,2,3,4,5,6,7,8,9])

    size = await self.list.add('l', [-1,0])
    self.assertEqual(size, 11)
    self.assertListEqual(await self.list.get_n('l'), [-1,0,1,2,3,4,5,6,7,8,9])


  async def test_get_head_tail(self):
    await self.list.create('l', type='int') 
    await self.list.add('l', [1,2,3,4,5,6,7,8,9,10], items_sorted=True)

    self.assertEqual(await self.list.get_head('l'), 1)
    self.assertEqual(await self.list.get_tail('l'), 10)


  async def test_get_n(self):
    await self.list.create('l', type='int') 
    await self.list.add('l', [2,3,1,5,6,4,7,8,10,9])

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


  async def test_remove(self):
    await self.list.create('rmv', type='int')
    await self.list.add('rmv', [0,1,2,3,4,5,6,7,8,9,10], items_sorted=True)
    
    await self.list.remove('rmv', start=0, stop=3)
    self.assertListEqual(await self.list.get_n('rmv'), [3,4,5,6,7,8,9,10])

    await self.list.remove('rmv', start=2, stop=-2)
    self.assertListEqual(await self.list.get_n('rmv'), [3,4,9,10])

    await self.list.remove('rmv', start=-4, stop=3)
    self.assertListEqual(await self.list.get_n('rmv'), [10])


  async def test_remove_if(self):
    await self.list.create('rmv', type='int')
    await self.list.add('rmv', [0,1,2,5,5,5,6,7,8,9,7,7,10])
    # [0,1,2,5,5,5,6,7,7,7,8,9,10]
    
    await self.list.remove_if_eq('rmv', start=0, stop=7, val=5)
    self.assertListEqual(await self.list.get_n('rmv'), [0,1,2,6,7,7,7,8,9,10])

    await self.list.remove_if_eq('rmv', start=-7, val=7)
    self.assertListEqual(await self.list.get_n('rmv'), [0,1,2,6,8,9,10])
  

  async def test_intersect_int(self):
    await self.list.create('i1', type='int')
    await self.list.create('i2', type='int')

    await self.list.add('i1', [0,1,2,5,5,5,6,7,8,9,10], items_sorted=True)
    await self.list.add('i2', [0,1,2,5,5,5,6,7], items_sorted=True)

    self.assertListEqual(await self.list.intersect('i1', 'i2'), [0,1,2,5,5,5,6,7])
    self.assertListEqual(await self.list.intersect('i1', 'i2', l1_stop=6), [0,1,2,5,5,5])
    self.assertListEqual(await self.list.intersect('i1', 'i2', l2_stop=3), [0,1,2])
    self.assertListEqual(await self.list.intersect('i1', 'i2', l1_start=3, l1_stop=6, l2_stop=-2), [5,5,5])


  async def test_intersect_str(self):
    await self.list.create('s1', type='str')
    await self.list.create('s2', type='str')

    await self.list.add('s1', ['apple', 'cider', 'painful', 'tequila', 'yes'], items_sorted=True)
    await self.list.add('s2', ['apple', 'beer', 'cider', 'no', 'painful', 'tequila'], items_sorted=True)

    self.assertListEqual(await self.list.intersect('s1', 's2'), ['apple', 'cider', 'painful', 'tequila'])
    self.assertListEqual(await self.list.intersect('s1', 's2', l1_stop=2, l2_stop=-3), ['apple', 'cider'])
    self.assertListEqual(await self.list.intersect('s1', 's2', l1_start=2), ['painful', 'tequila'])
    self.assertListEqual(await self.list.intersect('s1', 's2', l1_start=2, l2_start=4), ['painful', 'tequila'])
  

  async def test_intersect_new_int(self):
    await self.list.create('i1', type='int')
    await self.list.create('i2', type='int')

    await self.list.add('i1', [0,1,2,5,5,5,6,7,8,9,10], items_sorted=True)
    await self.list.add('i2', [0,1,2,5,5,5,6,7], items_sorted=True)

    await self.list.intersect('i1', 'i2', new_list_name='i3')
    await self.list.intersect('i1', 'i2', l2_stop=3, new_list_name='i4')

    self.assertListEqual(await self.list.get_n('i3'), [0,1,2,5,5,5,6,7])
    self.assertListEqual(await self.list.get_n('i4'), [0,1,2])

    self.assertListEqual(await self.list.intersect('i3', 'i4'), [0,1,2,])


  async def test_intersect_errors(self):
    await self.list.create('i', type='int')
    await self.list.create('s', type='str')
    await self.list.create('f', type='float')

    with self.assertRaises(ResponseError):
      await self.list.intersect('i', 's')
      await self.list.intersect('s', 'f')
      await self.list.intersect('f', 'i')

    with self.assertRaises(ResponseError):
      await self.list.intersect('i', 's', new_list_name='x')
      await self.list.intersect('s', 'f', new_list_name='x')
      await self.list.intersect('f', 'i', new_list_name='x')
