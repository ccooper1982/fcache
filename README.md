# fcache

fcache is a FlatBuffers cache over WebSockets.

There is support for key-values, with containers (Arrays, Lists) coming soon.

FlatBuffers offer zero-copy deserialising: when the server receives data, it can deserialise without an intermediate step which requires allocating memory (as with ProtoBuf), and is considerably more compact than JSON.

fcache docs available [here](https://ccooper1982.github.io/fcache/).

<br/>


# Python Client
The client API hides the FlatBuffer details:

```py
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


if __name__ == "__main__":
  asio.run(kv())
```

<br/>

## KV

- Keys must be a string
- Values can be:
  - string, int, unsigned int, float, bool
  - list/array of the above

A list of values can only contain values of the same type (i.e. a list of integers, a list of strings, but not a list containing different types).

<br/>

## Build

```
git clone git@github.com:ccooper1982/fcache.git
cd fcache
./build.sh
```

Binary is in `server/release`.

<br/>

## Run


|Param|Default|Cmd Line|
|---|---|---|
|IP|127.0.0.1|`--ip`|
|Port|1987|`--port / -p`|
|Max Payload|2048|`--maxPayload`|

<br/>

Override IP and port:

`./fcache --ip=192.168.0.10 -p 4321`

Override max payload:

`./fcache --maxPayload=16384`
