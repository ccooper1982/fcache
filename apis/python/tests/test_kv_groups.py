import unittest
from base import KvTest
from fc.common import ResponseError


class KVGroups(KvTest):
  """
  This tests mininmal with groups because the group keys are stored exactly as non-group keys.
  It checks basics of get/set, etc and nonexistent groups are handled.
  """
  async def test_set_get(self):
    await self.kv.set({'name':'Bob', 'age':25}, group='g1')
    await self.kv.set({'name':'Fred', 'age':50}, group='g2')

    g1 = await self.kv.get(keys=['name','age'], group='g1')
    g2 = await self.kv.get(keys=['name','age'], group='g2')

    self.assertDictEqual(g1, {'name':'Bob', 'age':25})
    self.assertDictEqual(g2, {'name':'Fred', 'age':50})

  
  async def test_add(self):
    await self.kv.add({'name':'Bob', 'age':25}, group='g1')
    kv = await self.kv.get(keys=['name','age'], group='g1')
    self.assertDictEqual(kv, {'name':'Bob', 'age':25})
    
    await self.kv.add({'name':'Bobby', 'age':250}, group='g1')
    kv = await self.kv.get(keys=['name','age'], group='g1')
    self.assertDictEqual(kv, {'name':'Bob', 'age':25}) # no change


  async def test_set_overwrite(self):
    await self.kv.set({'name':'Bob', 'age':25}, group='g1')
    await self.kv.set({'name':'Fred', 'age':50}, group='g2')

    g1 = await self.kv.get(keys=['name','age'], group='g1')
    g2 = await self.kv.get(keys=['name','age'], group='g2')

    self.assertDictEqual(g1, {'name':'Bob', 'age':25})
    self.assertDictEqual(g2, {'name':'Fred', 'age':50})

    await self.kv.set({'name':'Bob', 'age':250}, group='g1')
    await self.kv.set({'name':'Freddy', 'age':50}, group='g2')

    g1 = await self.kv.get(keys=['name','age'], group='g1')
    g2 = await self.kv.get(keys=['name','age'], group='g2')

    self.assertDictEqual(g1, {'name':'Bob', 'age':250})
    self.assertDictEqual(g2, {'name':'Freddy', 'age':50})


  async def test_get_all(self):
    await self.kv.set({'name':'Bob', 'age':25}, group='g1')
    await self.kv.set({'name':'Fred', 'age':50}, group='g2')

    self.assertDictEqual(await self.kv.get_all('g1'), {'name':'Bob', 'age':25})
    self.assertDictEqual(await self.kv.get_all('g2'), {'name':'Fred', 'age':50})


  async def test_get_key(self):
    await self.kv.set({'name':'Bob', 'age':25}, group='g1')
    await self.kv.set({'name':'Fred', 'age':50}, group='g2')

    self.assertEqual(await self.kv.get_key('name','g1'), 'Bob')
    self.assertEqual(await self.kv.get_key('name','g2'), 'Fred')


  async def test_get_keys(self):
    await self.kv.set({'a':'A', 'b':'B', 'c':'C'}, group='g1')
    self.assertDictEqual(await self.kv.get_keys(['a','c'],'g1'), {'a':'A','c':'C'})


  async def test_count(self):
    await self.kv.set({'name':'Bob', 'age':25}, group='g1')
    await self.kv.set({'name':'Fred', 'age':50, 'city':'London'}, group='g2')
    
    self.assertEqual(await self.kv.count(group='g1'), 2)
    self.assertEqual(await self.kv.count(group='g2'), 3)


  async def test_contains(self):
    await self.kv.set({'name':'Bob', 'age':25, 'city':'London'}, group='g1')
    
    rsp = list(await self.kv.contains(['name', 'age'], group='g1'))
    rsp.sort()

    self.assertTrue(rsp == ['age','name'])

    rsp = list(await self.kv.contains(['city'], group='g1'))
    rsp.sort()
    self.assertTrue(rsp == ['city'])

    self.assertTrue(await self.kv.contains(['name', '___'], group='g1') == ['name'])


  async def test_clear(self):
    await self.kv.set({'name':'Bob', 'age':25}, group='g1')
    await self.kv.set({'name':'Fred', 'age':50}, group='g2')
    await self.kv.set({'name':'Gary', 'age':20}, group='g3')

    g1 = await self.kv.get(keys=['name','age'], group='g1')
    g2 = await self.kv.get(keys=['name','age'], group='g2')

    self.assertDictEqual(g1, {'name':'Bob', 'age':25})
    self.assertDictEqual(g2, {'name':'Fred', 'age':50})

    # clear g1 then set again
    await self.kv.clear_group('g1', delete_group=False)
    self.assertEqual(await self.kv.count(group='g1'), 0)

    await self.kv.set({'email':'b@a.com', 'city':'Paris'}, group='g1')
    self.assertEqual(await self.kv.count(group='g1'), 2)

    await self.kv.clear_groups(delete_groups=False)
    self.assertEqual(await self.kv.count(group='g1'), 0)
    self.assertEqual(await self.kv.count(group='g2'), 0)
    self.assertEqual(await self.kv.count(group='g3'), 0)

    # recreate groups/keys then delete all
    await self.kv.set({'name':'Bob', 'age':25}, group='g1')
    await self.kv.clear_groups()
    self.assertEqual(await self.kv.get(key='name',group='g1'), None)


  async def test_remove(self):
    await self.kv.set({'name':'Bob', 'age':25, 'city':'Paris', 'active':True}, group='g1')

    await self.kv.remove(key='name',group='g1')
    self.assertDictEqual(await self.kv.contains(['name'], group='g1'),{})

    await self.kv.remove(keys=['age','city'],group='g1')
    self.assertDictEqual(await self.kv.contains(['age','city'], group='g1'),{})

    self.assertEqual(await self.kv.count(group='g1'), 1)


  async def test_clear_set(self):
    data1 = {'name':'Bob', 'age':25, 'city':'Paris', 'active':True}
    data2 = {'name':'Old Bob', 'age':250, 'city':'London', 'active':False}

    await self.kv.set(data1, group='g1')
    self.assertDictEqual(await self.kv.get(keys=['name','age','city','active'], group='g1'), data1)

    await self.kv.clear_set(data2, group='g1')
    self.assertDictEqual(await self.kv.get(keys=['name','age','city','active'], group='g1'), data2)


  async def test_key_not_exist(self):
    await self.kv.set({'a':'b'}, group='g1')
    self.assertIsNone(await self.kv.get(key='___', group='g1'))

  
  async def test_group_not_exist(self):
    self.assertIsNone(await self.kv.get(key='irrelevant', group='_dont_exist'))


if __name__ == "__main__":
  unittest.main()