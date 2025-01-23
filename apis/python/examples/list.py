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
        

async def get():
  if (client := await connect()) is None:
    return

  try:
    list = List(client)

    await list.create(name='list1', type='int', failOnDuplicate=False)
    await list.add_head(name='list1', items=[1,2,5,6])
    await list.add(name='list1', items=[3,4], pos=2)
    print(await list.get_n('list1', start=0))

  except:
    print('Query failed')



if __name__ == "__main__":
  async def run():
    for f in [get]:
      print(f'---- {f.__name__} ----')
      await f()
  
  asio.run(run())