import asyncio as asio
import sys
sys.path.append('../')
from fc.client import FcClient
from fc.kv import KV


async def connect() -> FcClient:
  try:
    client = FcClient()
    await client.open('ws://127.0.0.1:1987')
  except:
    print ('Failed to connect')
    client = None
  return client


async def kv():
  if (client := await connect()) is None:
    return
  
  # create API object for KV commands
  kv = KV(client)

  data = {'user':'user1', 'age':25, 'active':True}
  await kv.set(data)

  # get single key, returns the value
  age = await kv.get(key='age')
  print(f'Age: {age}')

  # get multiple keys, returns dict
  rsp = await kv.get(keys=['user', 'active'])
  print(f"User: {rsp['user']}, Active: {rsp['active']}")


if __name__ == "__main__":
  asio.run(kv())