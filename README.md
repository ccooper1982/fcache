# fcache

fcache is a data cache, using Google's FlatBuffers over WebSockets.

There is support for key-values, with containers (Arrays, Lists) coming soon.

FlatBuffers offer zero-copy deserialising: when the server receives data, it can deserialise without an intermediate step which requires allocating memory (as with ProtoBuf), and is considerably more compact than JSON.

<br/>

# Python Client
The client API hides the FlatBuffer details:


```py
import asyncio as asio
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
```
