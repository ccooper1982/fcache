import unittest
from base import KvTest
from fc.common import ResponseError


class KV(KvTest):
  # async def test_set_get_key(self):
  #   input = {'k1':123, 'k2':True, 'k3':123.5, 'k4':'stringaling'}
  #   await self.kv.set(input)

  #   out = await self.kv.get(key='k1')
  #   self.assertIsInstance(out, int)
  #   self.assertEqual(out, input['k1'])

  #   out = await self.kv.get(key='k2')
  #   self.assertIsInstance(out, bool)
  #   self.assertEqual(out, input['k2'])

  #   out = await self.kv.get(key='k3')
  #   self.assertIsInstance(out, float)
  #   self.assertEqual(out, input['k3'])

  #   out = await self.kv.get(key='k4')
  #   self.assertIsInstance(out, str)
  #   self.assertEqual(out, input['k4'])


  # async def test_set_get_keys(self):
  #   input = {'k1':123, 'k2':True, 'k3':123.5, 'k4':'stringaling'}
  #   keys = ['k1','k2','k3','k4']

  #   await self.kv.set(input)
  #   out = await self.kv.get(keys=keys)

  #   self.assertDictEqual(input, out)

  #   for k in keys:
  #     self.assertEqual(type(out[k]), type(input[k]))

  
  # async def test_add(self):
  #   input = {'k1':123, 'k2':True, 'k3':123.5, 'k4':'stringaling'}
  #   keys = ['k1','k2','k3','k4']

  #   await self.kv.set(input)
  #   out = await self.kv.get(keys=keys)
  #   self.assertDictEqual(input, out)

  #   # attempt add() of existing key should do nothing
  #   await self.kv.add({'k1':321})
  #   out = await self.kv.get(key='k1')
  #   self.assertEqual(input['k1'], out)


  # async def test_set_overwrite(self):
  #   input = {'k1':123, 'k2':True, 'k3':123.5, 'k4':'stringaling'}
  #   keys = ['k1','k2','k3','k4']

  #   await self.kv.set(input)
  #   out = await self.kv.get(keys=keys)
  #   self.assertDictEqual(input, out)

  #   # overwrite k1
  #   await self.kv.set({'k1':321})
  #   out = await self.kv.get(key='k1')
  #   self.assertEqual(321, out)


  # async def test_count(self):
  #   input = {'k1':123, 'k2':True, 'k3':123.5, 'k4':'stringaling'}
    
  #   self.assertEqual(await self.kv.count(), 0)
  #   await self.kv.set(input)
  #   self.assertEqual(await self.kv.count(), len(input))


  # async def test_contains(self):
  #   input = {'k1':123, 'k2':True, 'k3':123.5, 'k4':'stringaling'}
  #   keys = ['k1','k2','k3','k4']

  #   await self.kv.set(input)
  #   resultSet = await self.kv.contains(keys)
  #   resultListSorted = list(resultSet)
  #   resultListSorted.sort()
  #   self.assertListEqual(list(input.keys()), resultListSorted)

  #   # non-existent
  #   resultSet = await self.kv.contains(['k1', '_'])
  #   self.assertListEqual(['k1'], list(resultSet))


  # async def test_clear(self):
  #   input = {'k1':123, 'k2':True, 'k3':123.5, 'k4':'stringaling'}
    
  #   await self.kv.set(input)
  #   self.assertEqual(await self.kv.count(), len(input))

  #   await self.kv.clear()
  #   self.assertEqual(await self.kv.count(), 0)


  # async def test_remove(self):
  #   data = {'k1':123, 'k2':True, 'k3':123.5, 'k4':'stringaling'}
    
  #   await self.kv.set(data)
  #   out = await self.kv.get(keys=list(data.keys()))
  #   self.assertDictEqual(data, out)

  #   await self.kv.remove(key='k1')
  #   out = await self.kv.get(keys=list(data.keys()))
  #   self.assertDictEqual({'k2':True, 'k3':123.5, 'k4':'stringaling'}, out)

  #   await self.kv.remove(keys=['k2', 'k3'])
  #   out = await self.kv.get(keys=list(data.keys()))
  #   self.assertDictEqual({'k4':'stringaling'}, out)
  

  # async def test_clear_set(self):
  #   input1 = {'k1':123, 'k2':True, 'k3':123.5, 'k4':'stringaling'}
  #   keys1 = ['k1','k2','k3','k4']
  #   input2 = {'k10':'x', 'k11':False}

  #   allKeys = keys1[:]
  #   allKeys.extend(['k10', 'k11'])

  #   await self.kv.set(input1)
  #   out = await self.kv.get(keys=keys1)
  #   self.assertDictEqual(input1, out)

  #   # clear input1 then set input2
  #   await self.kv.clear_set(input2)
  #   await self.kv.set(input2)
  #   # request all keys from input1 and input2.
  #   # should only receive input2 keys (since input1 cleared)
  #   out = await self.kv.get(keys=allKeys)
  #   self.assertDictEqual(input2, out)


  ## Errors
  async def test_set_list_types(self):
    with self.assertRaises(ValueError):
      await self.kv.set({'strings':['hello', 123]})


if __name__ == "__main__":
  unittest.main()