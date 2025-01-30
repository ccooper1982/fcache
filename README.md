# fcache

fcache is a FlatBuffers cache over WebSockets.

There is support for key-values, unsorted lists and sorted lists with more coming soon.

FlatBuffers offer zero-copy deserialising: when the server receives data, it can deserialise without an intermediate step which requires allocating memory (as with ProtoBuf), and is considerably more compact than JSON.

fcache docs available [here](https://ccooper1982.github.io/fcache/).

<br/>


# Python Client
The client API hides the FlatBuffers details:

```py
import fc
from fc.kv import KV
from fc.list import UnsortedList


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

  # get single key, returns the value
  age = await kv.get(key='level')
  print(f'Age: {age}')

  # get multiple keys, returns dict
  rsp = await kv.get(keys=['player', 'active'])
  print(f"Player: {rsp['player']}, Active: {rsp['active']}")

  print(await kv.get(key='perks'))


  # Unsorted list API. There's also SortedList
  lst = UnsortedList(client)

  await lst.create('scores', type='int')
  await lst.add_head('scores', [1,2,3,5,5,6,8])
  await lst.get_n('scores', start=3, count=2) # [5,5]
  await lst.get_range('scores', start=1, stop=-2)  # [2,3,5,5]
  await lst.remove_if_eq('scores', val=5)  # [1,2,3,6,8]


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

[Read more](https://ccooper1982.github.io/fcache/kv/).

<br/>

## Lists

Sorted and unsorted lists of integers, string and floats are supported. Sorted lists can also be intersected.

[Read more](https://ccooper1982.github.io/fcache/lists/).

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
|Max Payload|16384 (bytes)|`--maxPayload`|

<br/>

Override IP and port:

`./fcache --ip=192.168.0.10 -p 4321`

Override max payload:

`./fcache --maxPayload=16384`
