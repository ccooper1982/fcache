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

    await list.create(name='list1', type='int', failOnDuplicate=False)

    await list.add(name='list1', items=[1,2,3,4])

  except:
    print('Query failed')
        


if __name__ == "__main__":
  async def run():
    for f in [create]:
      print(f'---- {f.__name__} ----')
      await f()
  
  asio.run(run())