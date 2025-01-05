import asyncio as asio
import sys
sys.path.append('../')
import fc
from fc.kv import KV


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
  
  # create API object for KV commands
  kv = KV(client)

  await kv.set({'user':'user1', 'age':25, 'active':True})

  # get single key, returns the value
  age = await kv.get(key='age')
  print(f'Age: {age}')

  # get multiple keys, returns dict
  rsp = await kv.get(keys=['user', 'active'])
  print(f"User: {rsp['user']}, Active: {rsp['active']}")


if __name__ == "__main__":
  asio.run(kv())