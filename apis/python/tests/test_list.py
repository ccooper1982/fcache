from base import UnsortedListTest
from fc.common import ResponseError


class UnsortedList(UnsortedListTest):
  async def test_create(self):
    # TODO add more checks when exists() available
    await self.list.create('l', type='int', fail_on_duplicate=False)
    await self.list.create('l', type='int', fail_on_duplicate=False)

    with self.assertRaises(ResponseError):
      await self.list.create('l', type='int', fail_on_duplicate=True)


  async def test_add_head(self):
    await self.list.create('l', type='int')

    size = await self.list.add_head('l', [5])
    self.assertEqual(size, 1)
    self.assertListEqual(await self.list.get_n('l'), [5])

    size = await self.list.add_head('l', [4])
    self.assertEqual(size, 2)
    self.assertListEqual(await self.list.get_n('l'), [4,5])

    size = await self.list.add_head('l', [1,2,3])
    self.assertEqual(size, 5)
    self.assertListEqual(await self.list.get_n('l'), [1,2,3,4,5])


  async def test_add_tail(self):
    await self.list.create('l', type='int')

    size = await self.list.add_tail('l', [5])
    self.assertEqual(size, 1)
    self.assertListEqual(await self.list.get_n('l'), [5])

    size = await self.list.add_tail('l', [6,7,8])
    self.assertEqual(size, 4)
    self.assertListEqual(await self.list.get_n('l'), [5,6,7,8])


  async def test_add(self):
    await self.list.create('l', type='int')

    # same as add_head
    size = await self.list.add('l', [1,2,3])
    self.assertEqual(size, 3)
    self.assertListEqual(await self.list.get_n('l'), [1,2,3])

    # same as add_tail
    size = await self.list.add('l', [7,8,9], pos=3)
    self.assertEqual(size, 6)
    self.assertListEqual(await self.list.get_n('l'), [1,2,3,7,8,9])

    # insert mid
    size = await self.list.add('l', [4,5,6], pos=3)
    self.assertEqual(size, 9)
    self.assertListEqual(await self.list.get_n('l'), [1,2,3,4,5,6,7,8,9])

    # as add_head() but with items already in list
    size = await self.list.add('l', [-1,0], pos=0)
    self.assertEqual(size, 11)
    self.assertListEqual(await self.list.get_n('l'), [-1,0,1,2,3,4,5,6,7,8,9])

    
  async def test_add_negative(self):
    await self.list.create('l', type='int')

    # pos out of bounds, adds to end
    size = await self.list.add('l', [7,8,9], pos=3)
    self.assertEqual(size, 3)
    self.assertListEqual(await self.list.get_n('l'), [7,8,9])

    size = await self.list.add('l', [1,3], pos=-3)
    self.assertEqual(size, 5)
    self.assertListEqual(await self.list.get_n('l'), [1,3,7,8,9])

    size = await self.list.add('l', [2], pos=-4)
    self.assertEqual(size, 6)
    self.assertListEqual(await self.list.get_n('l'), [1,2,3,7,8,9])

    size = await self.list.add('l', [4,5,6], pos=-3)
    self.assertEqual(size, 9)
    self.assertListEqual(await self.list.get_n('l'), [1,2,3,4,5,6,7,8,9])

    size = await self.list.add('l', [8], pos=-1)
    self.assertEqual(size, 10)
    self.assertListEqual(await self.list.get_n('l'), [1,2,3,4,5,6,7,8,8,9])


  async def test_get_head_tail(self):
    await self.list.create('l', type='int') 
    await self.list.add('l', [1,2,3,4,5,6,7,8,9,10])

    self.assertEqual(await self.list.get_head('l'), 1)
    self.assertEqual(await self.list.get_tail('l'), 10)


  async def test_get_n(self):
    await self.list.create('l', type='int') 
    await self.list.add('l', [1,2,3,4,5,6,7,8,9,10])

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
    await self.list.create('l', type='int') 
    await self.list.add('l', [1,2,3,4,5,6,7,8,9,10])

    self.assertListEqual(await self.list.get_n_reverse('l',start=0), [10,9,8,7,6,5,4,3,2,1])
    self.assertListEqual(await self.list.get_n_reverse('l',start=5), [5,4,3,2,1])
    self.assertListEqual(await self.list.get_n_reverse('l',start=9), [1])
    self.assertListEqual(await self.list.get_n_reverse('l',start=7, count=2), [3,2])
    self.assertListEqual(await self.list.get_n_reverse('l',start=7, count=20), [3,2,1])

  
  async def test_get_range_forward(self):
    await self.list.create('l', type='int') 
    await self.list.add_head('l', [1,2,3,4,5,6,7,8,9,10])

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
    await self.list.create('l', type='int') 
    await self.list.add_head('l', [1,2,3,4,5,6,7,8,9,10])

    # everything from tail
    self.assertListEqual (await self.list.get_range_reverse('l', start=0), [10,9,8,7,6,5,4,3,2,1])
    # everything from mid
    self.assertListEqual (await self.list.get_range_reverse('l', start=7), [3,2,1])
    # everything from head
    self.assertListEqual (await self.list.get_range_reverse('l', start=9), [1])

    # positive positions
    self.assertListEqual (await self.list.get_range_reverse('l', start=3, stop=5), [7,6])
    # stop negative
    self.assertListEqual (await self.list.get_range_reverse('l', start=3, stop=-2), [7,6,5,4,3])
    # start negative
    self.assertListEqual (await self.list.get_range_reverse('l', start=-8, stop=4), [8,7])
    # both negative
    self.assertListEqual (await self.list.get_range_reverse('l', start=-8, stop=-5), [8,7,6])


  async def test_get_range_bounds(self):
    await self.list.create('l', type='int') 
    await self.list.add_head('l', [1,2,3,4,5])

    # start bounds: no error, just empty result
    self.assertListEqual (await self.list.get_range('l', start=20), [])
    self.assertListEqual (await self.list.get_range_reverse('l', start=20), [])
    
    # end bounds: gets to end
    self.assertListEqual (await self.list.get_range('l', start=0, stop=20), [1,2,3,4,5])
    self.assertListEqual (await self.list.get_range_reverse('l', start=0, stop=20), [5,4,3,2,1])


  async def test_remove(self):
    await self.list.create('rmv', type='int')
    await self.list.add_head('rmv', [0,1,2,3,4,5,6,7,8,9,10])
    
    await self.list.remove('rmv', start=0, stop=3)
    self.assertListEqual(await self.list.get_n('rmv'), [3,4,5,6,7,8,9,10])

    await self.list.remove('rmv', start=2, stop=-2)
    self.assertListEqual(await self.list.get_n('rmv'), [3,4,9,10])

    await self.list.remove('rmv', start=-4, stop=3)
    self.assertListEqual(await self.list.get_n('rmv'), [10])


  async def test_remove_if_int(self):
    await self.list.create('rmv', type='int')
    await self.list.add('rmv', [0,1,2,5,5,5,6,7,8,9,7,7,10])
    
    size = await self.list.remove_if_eq('rmv', start=0, stop=7, val=5)
    self.assertEqual(size, 10)
    self.assertListEqual(await self.list.get_n('rmv'), [0,1,2,6,7,8,9,7,7,10])

    size = await self.list.remove_if_eq('rmv', start=-7, val=7)
    self.assertEqual(size, 7)
    self.assertListEqual(await self.list.get_n('rmv'), [0,1,2,6,8,9,10])


  async def test_remove_if_str(self):
    await self.list.create('rmv', type='str')
    await self.list.add('rmv', ['A','B','B','C','D','D','D','E'])

    await self.list.remove_if_eq('rmv', val='B')
    self.assertListEqual(await self.list.get_n('rmv'),['A','C','D','D','D','E'])

    await self.list.remove_if_eq('rmv', stop=3, val='D')
    self.assertListEqual(await self.list.get_n('rmv'),['A','C','D','D','E'])

    await self.list.remove_if_eq('rmv', start=2, stop=4, val='D')
    self.assertListEqual(await self.list.get_n('rmv'),['A','C','E'])


  async def test_remove_head(self):
    data = [0,1,2,3,4,5,6,7,8,9]

    await self.list.create('l', type='int')
    await self.list.add('l', data)

    for i in range(10):
      await self.list.remove_head('l')
      self.assertListEqual(await self.list.get_n('l'), data[i+1:])

    self.assertListEqual(await self.list.get_n('l'), [])


  async def test_remove_tail(self):
    data = [0,1,2,3,4,5,6,7,8,9]

    await self.list.create('l', type='int')
    await self.list.add('l', data)

    for i in range(10):
      await self.list.remove_tail('l')
      self.assertListEqual(await self.list.get_n('l'), data[:-(i+1)])

    self.assertListEqual(await self.list.get_n('l'), [])


  async def test_set(self):
    await self.list.create('l', type='int')
    await self.list.add('l', [0,0,1,1,1,20,20,30,30])

    await self.list.set('l', [2,2,3,3], pos=5)
    self.assertListEqual(await self.list.get_n('l'), [0,0,1,1,1,2,2,3,3])

    await self.list.set('l', [4], pos=-1)
    self.assertListEqual(await self.list.get_n('l'), [0,0,1,1,1,2,2,3,4])

    await self.list.set('l', [3,4,5], pos=-3)
    self.assertListEqual(await self.list.get_n('l'), [0,0,1,1,1,2,3,4,5])


  async def test_set_errors(self):
    await self.list.create('l', type='int')
    
    # target list is empty
    await self.list.set('l', [0,0,0], pos=0)
    await self.list.set('l', [0,0,0], pos=10)

    await self.list.add('l', [0,0,1,1,1,20,20,30,30])
    
    # partially inbounds
    await self.list.set('l', [20,20,30,30], pos=7)
    self.assertListEqual(await self.list.get_n('l'), [0,0,1,1,1,20,20,20,20])

    # out of bounds
    await self.list.set('l', [0,0,0], pos=50)
    self.assertListEqual(await self.list.get_n('l'), [0,0,1,1,1,20,20,20,20])


  async def test_size(self):
    await self.list.create('l', type='int')
    
    self.assertEqual(await self.list.size('l'), 0)
    
    await self.list.add('l', [0,1,2,3,4,5,6,7,8,9])
    self.assertEqual(await self.list.size('l'), 10)

    await self.list.remove('l', start=0, stop=5)
    self.assertEqual(await self.list.size('l'), 5)

    await self.list.remove('l', start=0, stop=5)
    self.assertEqual(await self.list.size('l'), 0)


  async def test_info(self):
    await self.list.create('i', type='int')
    await self.list.create('s', type='str')
    await self.list.create('f', type='float')
    
    (size, type, sorted) = await self.list.info('i')
    self.assertEqual(size, 0)
    self.assertEqual(type, 'int')
    self.assertFalse(sorted)

    (size, type, sorted) = await self.list.info('s')
    self.assertEqual(size, 0)
    self.assertEqual(type, 'str')
    self.assertFalse(sorted)

    (size, type, sorted) = await self.list.info('f')
    self.assertEqual(size, 0)
    self.assertEqual(type, 'float')
    self.assertFalse(sorted)

    await self.list.add('i', [0,1,2])
    await self.list.add('s', ['a','b','c'])
    await self.list.add('f', [1.1, 1.2, 1.3])

    (size, type, sorted) = await self.list.info('i')
    self.assertEqual(size, 3)
    (size, type, sorted) = await self.list.info('s')
    self.assertEqual(size, 3)
    (size, type, sorted) = await self.list.info('f')
    self.assertEqual(size, 3)


  # TODO delete, delete_all when exists() implemented