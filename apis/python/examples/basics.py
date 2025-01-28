import asyncio as asio
import sys
sys.path.append('../')
import fc
from fc.kv import KV
from fc.list import UnsortedList, SortedList


async def connect() -> fc.Client:
  try:
    client = await fc.fcache(uri='ws://127.0.0.1:1987')
    # or: client = await fc.fcache(ip='127.0.0.1', port=1987)
  except:
    print ('Failed to connect')
    client = None
  return client


async def kv():
  if (client := await connect()) is None:
    return
  
  # create API object for KV functions
  kv = KV(client)

  await kv.set({'player':'Monster',
                'level':25,
                'active':True,
                'perks':['Armour','Kilt']})

  # get single key, returns the value (or None if key not found)
  age = await kv.get(key='level')
  print(f'Age: {age}')

  # get multiple keys, returns dict
  rsp = await kv.get(keys=['player', 'active'])
  print(f"Player: {rsp['player']}, Active: {rsp['active']}")

  # get list
  print(await kv.get(key='perks'))


async def kv_blob():
  # Requires fcache is started with maxPayload to size of the cat image which is 11,030 bytes
  # ./fcache --maxPayload=16384

  if (client := await connect()) is None:
    return
  
  file_data = bytes()
  with open('cat.jpg', mode='rb') as file:
    file_data = file.read()

  kv = KV(client)

  await kv.set({'img_cat':file_data})

  if data := await kv.get(key='img_cat'):
    with open('cat_rsp.jpg', mode='wb') as file:
      file.write(data)
      print('Image written')


async def unsorted_lists():
  if (client := await connect()) is None:
    return
  
  # create API object for list functions
  list = UnsortedList(client)

  await list.delete_all()

  # create list for integers
  await list.create('scores', type='int')
  # add these items to head
  await list.add_head('scores', [25,35,45,55])
  # insert in between 35 and 45 (at position 2)
  await list.add('scores', [40], pos=2)
  # add two more to the tail
  await list.add_tail('scores', [60, 65])
  
  # get everything from the first item
  print(f"a. {await list.get_n('scores')}")
  # get everything in reverse
  print(f"b. {await list.get_n_reverse('scores')}")
  # # get the first 3 
  print(f"c. {await list.get_n('scores', count=3)}")
  # # get the last 2
  print(f"d. {await list.get_n('scores', start=5)}")
  # get middle 3 with range (could also use get_n())
  print(f"e. {await list.get_range('scores', start=2, stop=5)}")
  # get middle 5 in reverse, using negative index
  print(f"f. {await list.get_range_reverse('scores', start=1, stop=-1)}")


  # add tail
  await list.create('numbers', type='int')
  await list.add_tail('numbers', [60, 65])
  print(await list.get_n('numbers'))


  # list of strings
  await list.create('names', type='str')
  # alternative to add_head()
  await list.add('names', ['Arya', 'Fiona'], pos=0)
  print(await list.get_n('names'))
  await list.add('names', ['David', 'Bob', 'Charlie'], pos=1)
  print(await list.get_n('names'))
  await list.add('names', ['Emma'], pos=3)
  print(await list.get_n('names'))


  # floats
  await list.create('prices', type='float')
  
  await list.add_head('prices', [123.5, 88.5])
  print(await list.get_n('prices'))

  await list.add('prices', [100.0], pos=1)
  print(await list.get_n('prices'))


  # get_n
  await list.create('list', type='str')
  await list.add_head('list', ['A', 'B', 'C', 'D', 'E'])
  
  print('get_n()')
  print(await list.get_n('list'))
  print(await list.get_n('list', start=3))
  print(await list.get_n('list', start=1, count=2))
  print(await list.get_n('list', count=3))

  print('get_n_reverse()')
  print(await list.get_n_reverse('list'))
  print(await list.get_n_reverse('list', start=3))
  print(await list.get_n_reverse('list', start=1, count=2))
  print(await list.get_n_reverse('list', count=3))


  # get_range
  print('get_range')
  print(await list.get_range('list', start=0))
  print(await list.get_range('list', start=2))
  print(await list.get_range('list', start=-2))
  print(await list.get_range('list', start=1, stop=-2))
  print(await list.get_range('list', start=-4, stop=-1))
  
  print('get_range_reverse')
  print(await list.get_range_reverse('list', start=0))
  print(await list.get_range_reverse('list', start=2))
  print(await list.get_range_reverse('list', start=-2))
  print(await list.get_range_reverse('list', start=1, stop=-2))
  print(await list.get_range_reverse('list', start=-4, stop=-1))
  
  # remove
  print('remove')
  await list.create('rmv', type='int')
  await list.add_head('rmv', [0,1,2,3,4,5,6,7,8,9,10])
  print(await list.get_n('rmv'))
  
  await list.remove('rmv', start=0, stop=3)
  print(await list.get_n('rmv'))

  await list.remove('rmv', start=1, stop=-1)
  print(await list.get_n('rmv'))

  await list.remove('rmv')
  print(await list.get_n('rmv'))

  await list.add_head('rmv', [0,1,2,3,4,5,6,7,8,9,10])
  print(await list.get_n('rmv'))


  # remove if  
  print('remove if int')
  await list.create('rmv_if', type='int')
  await list.add_head('rmv_if', [0,1,2,5,5,5,6,7,8,9,7,7,10])
  print(await list.get_n('rmv_if'))
    
  await list.remove_if_eq('rmv_if', start=0, stop=7, val=5)
  print(await list.get_n('rmv_if'))

  # [0,1,2,6,7,8,9,7,7,10]
  await list.remove_if_eq('rmv_if', start=-3, val=7)
  print(await list.get_n('rmv_if'))


  print('remove if str')
  await list.create('rmv_if_str', type='str')
  await list.add_head('rmv_if_str', ['abba', 'metallica', 'abba', 'mj', 'abba', 'mj', 'mj'])
  print(await list.get_n('rmv_if_str'))
    
  await list.remove_if_eq('rmv_if_str', start=0, stop=3, val='abba')
  print(await list.get_n('rmv_if_str')) # ['metallica', 'mj', 'abba', 'mj',' mj']

  await list.remove_if_eq('rmv_if_str', start=2, val='mj')
  print(await list.get_n('rmv_if_str')) # ['metallica', 'mj', 'abba']


async def sorted_lists():
  if (client := await connect()) is None:
    return
  
  lst = SortedList(client)

  await lst.delete_all()

  await lst.create('scores', type='int')

  await lst.add('scores', [45,35,25,55])
  print(await lst.get_n('scores'))

  await lst.add('scores', [50,20,100,40,90])
  print(await lst.get_n('scores'))

  await lst.add('scores', [41,42], items_sorted=True)
  print(await lst.get_n('scores'))

  await lst.add('scores', [1,2,3], items_sorted=True)
  print(await lst.get_n('scores'))

  await lst.add('scores', [100,101,102], items_sorted=True)
  print(await lst.get_n('scores'))

  print(await lst.get_n('scores', start=5, count=4))
  print(await lst.get_n_reverse('scores', start=8, count=4))

  # list of strings
  await lst.create('names', type='str')
  await lst.add('names', ['Arya', 'Fiona'])
  print(await lst.get_n('names'))
  await lst.add('names', ['David', 'Bob', 'Charlie'])
  print(await lst.get_n('names'))
  await lst.add('names', ['Emma'])
  print(await lst.get_n('names'))


  # intersect int
  await lst.create('i1', type='int')
  await lst.create('i2', type='int')

  await lst.add('i1', [0,1,2,5,5,5,6,7,8,9,10], items_sorted=True)
  await lst.add('i2', [0,1,2,5,5,5,6,7], items_sorted=True)

  print(await lst.intersect('i1', 'i2'))
  print(await lst.intersect('i1', 'i2', l1_stop=6))
  print(await lst.intersect('i1', 'i2', l2_stop=3))
  print(await lst.intersect('i1', 'i2', l1_start=3, l1_stop=6, l2_stop=-2))

  #intersect str
  await lst.create('s1', type='str')
  await lst.create('s2', type='str')

  await lst.add('s1', ['apple', 'cider', 'painful', 'tequila', 'yes'], items_sorted=True)
  await lst.add('s2', ['apple', 'beer', 'cider', 'no', 'painful', 'tequila'], items_sorted=True)

  print(await lst.intersect('s1', 's2'))
  print(await lst.intersect('s1', 's2', l1_stop=2, l2_stop=-3))
  print(await lst.intersect('s1', 's2', l1_start=2))
  print(await lst.intersect('s1', 's2', l1_start=2, l2_start=4))


if __name__ == "__main__":
  async def run():
    #for f in [kv, kv_blob, unsorted_lists, sorted_lists]:
    for f in [unsorted_lists, sorted_lists]:
      print(f'---- {f.__name__} ----')
      await f()
  
  asio.run(run())