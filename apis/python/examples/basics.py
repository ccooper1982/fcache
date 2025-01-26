import asyncio as asio
import sys
sys.path.append('../')
import fc
from fc.kv import KV
from fc.list import List


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


async def lists():
  if (client := await connect()) is None:
    return
  
  # create API object for list functions
  list = List(client)

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
  
  await list.add_head('names', ['Adam','Charlie'])
  print(await list.get_n('names'))

  await list.add('names', ['Bob'], pos=1)
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


if __name__ == "__main__":
  async def run():
    for f in [kv, kv_blob, lists]:
      print(f'---- {f.__name__} ----')
      await f()
  
  asio.run(run())