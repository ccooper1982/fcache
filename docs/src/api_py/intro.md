# Overview

The client uses the connection to send/receive requests and responses, with a separate class for KV specific commands (and in the future, arrays, lists, etc).

The command classes require a `Client` object.

## Connect
```py
import fc

async def connect() -> fc.Client:
  try:
    client = await fc.fcache(uri='ws://127.0.0.1:1987')
    # or: client = await fc.fcache(ip='127.0.0.1', port=1987)
  except:
    print ('Failed to connect')
    client = None
  return client
```

## KV
```py
from fc.kv import KV

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
```


## Exceptions

- `ResponseError` : raised when a failed response is received
- `ValueError` : raised by the API before a request is sent
- `OSError` : if opening the connection fails
