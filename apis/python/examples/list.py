import asyncio as asio
import random
import sys
sys.path.append('../')
import fc
from fc.list import List


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
    list = List(client)

    await list.create(name='list1', type='int', failOnDuplicate=True)
    await list.create(name='list2', type='int', failOnDuplicate=True)
    await list.create(name='list3', type='int', failOnDuplicate=True)

    await list.delete_all()
    # await list.delete(names=['list1'])

    #await list.add_head(name='list1', items=[1,2,5,6])
    #await list.add(name='list1', items=[3,4], pos=2)
    #await list.add(name='list1', items=[7,8,9,10], pos=8)

    #await list.add_head(name='list1', items=[4,3,2,1])
    #await list.add_head(name='list1', items=[10,11,12])
    #await list.add_tail(name='list1', items=[100,101,102])

  except:
    print('Query failed')
        

async def get_count():
  if (client := await connect()) is None:
    return

  try:
    list = List(client)

    await list.delete_all()

    await list.create(name='list1', type='int', failOnDuplicate=False)
    await list.add_head(name='list1', items=[1,2,3,4,5,6])
    
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
    list = List(client)

    await list.delete_all()

    await list.create(name='list1', type='int', failOnDuplicate=False)
    await list.add_head('list1', items=[1,2,3,4,5,6,7,8,9,10])

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
    list = List(client)

    await list.delete_all()

    await list.create(name='list1', type='int', failOnDuplicate=False)
    await list.add_head('list1', items=[1,2,3,4,5,6,7,8,9,10])

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


if __name__ == "__main__":
  async def run():
    for f in [get_range_reverse]:
      print(f'---- {f.__name__} ----')
      await f()
  
  asio.run(run())