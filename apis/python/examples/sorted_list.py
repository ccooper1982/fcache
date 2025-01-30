import asyncio as asio
import random
import sys
sys.path.append('../')
import fc
from fc.list import SortedList


async def connect() -> fc.Client:
  try:
    return await fc.fcache('ws://127.0.0.1:1987')
  except:
    print ('Failed to connect')
  
  return None


async def create():
  if (client := await connect()) is None:
    return

  try:
    list = SortedList(client)

    await list.create('list1', type='int')
    await list.create('list2', type='int')
    await list.create('list3', type='int')

    await list.delete_all()

  except:
    print('Query failed')
        

async def get_count():
  if (client := await connect()) is None:
    return

  try:
    list = SortedList(client)

    await list.delete_all()

    await list.create('list1', type='int')
    await list.add('list1', [1,2,3,4,5,6])
    
    print(await list.get_n('list1', start=3))
    print(await list.get_n('list1', start=2, count=2))
    print(await list.get_n('list1', start=4))
    print(await list.get_n('list1', start=4, count=100))
    
    print(await list.get_n_reverse('list1', start=2, count=2))
    print(await list.get_n_reverse('list1', start=2, count=0))
  except Exception as e:
    print(f'Query failed: {e}')


async def get_range():
  if (client := await connect()) is None:
    return

  try:
    list = SortedList(client)

    await list.delete_all()

    await list.create('list1', type='int', fail_on_duplicate=False)
    await list.add('list1', [1,2,3,4,5,6,7,8,9,10])

    print(await list.get_range('list1', start=0, stop=2))
    print(await list.get_range('list1', start=0))
    print(await list.get_range('list1', start=3, stop=5))
    print(await list.get_range('list1', start=3))
    print(await list.get_range('list1', start=-1, stop=5)) # intentionally testing no results
    print(await list.get_range('list1', start=-6))
    print(await list.get_range('list1', start=-6, stop=-3))

  except Exception as e:
    print(f'Query failed: {e}')


async def get_range_reverse():
  if (client := await connect()) is None:
    return

  try:
    list = SortedList(client)

    await list.delete_all()

    await list.create('list1', type='int')
    await list.add('list1', [1,2,3,4,5,6,7,8,9,10])

    print(await list.get_range_reverse('list1', start=0, stop=2))
    print(await list.get_range_reverse('list1', start=0))
    print(await list.get_range_reverse('list1', start=3, stop=5))
    print(await list.get_range_reverse('list1', start=3))
    print(await list.get_range_reverse('list1', start=5, stop=-1))
    print(await list.get_range_reverse('list1', start=-6))
    print(await list.get_range_reverse('list1', start=-6, stop=-3))

    print(await list.get_range('list1', start=-1, stop=5)) # intentionally testing no results
    
  except Exception as e:
    print(f'Query failed: {e}')


async def remove():
  if (client := await connect()) is None:
    return

  try:
    list = SortedList(client)

    await list.delete_all()

    await list.create('l1', type='int')

    await list.add('l1', [0,1,2,3,4,5,6,7,8,9])    
    await list.remove('l1')

    await list.add('l1', [0,1,2,3,4,5,6,7,8,9])
    await list.remove('l1', start=0, stop=10)

  except Exception as e:
    print(f'Query failed: {e}')


async def intersect():
  if (client := await connect()) is None:
    return

  try:
    list = SortedList(client)

    await list.delete_all()

    await list.create('l1', type='int')
    await list.create('l2', type='int')

    await list.add('l1', [1,2,3,4,5,6,7,8,9,10], items_sorted=True)
    await list.add('l2', [1,2,9,10], items_sorted=True)
    
    print(await list.intersect('l1', 'l2'))
    print(await list.intersect('l1', 'l2', l2_start=-2))
  except Exception as e:
    print(f'Query failed: {e}')


async def intersect_new():
  if (client := await connect()) is None:
    return

  try:
    list = SortedList(client)

    await list.delete_all()

    await list.create('list1', type='int')
    await list.create('list2', type='int')

    await list.add('list1', [1,2,3,4,5,6,7,8,9,10], items_sorted=True)
    await list.add('list2', [1,2,9,10], items_sorted=True)
    
    await list.intersect('list1', 'list2', new_list_name='list3')
    print(await list.get_n('list3'))
  except Exception as e:
    print(f'Query failed: {e}')



if __name__ == "__main__":
  async def run():
    for f in [create, get_count, get_range, get_range_reverse, intersect, intersect_new]:
      print(f'---- {f.__name__} ----')
      await f()
  
  asio.run(run())