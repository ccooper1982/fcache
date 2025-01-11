# Overview

The `fc.Client` manages the connection, with a separate class for KV specific commands (and in the future, arrays, lists, etc).

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

!!! warning "List Values"
    - A list cannot be empty
    - If a key's value is a `list`, all elements must be the same type:

    ```py
    await kv.set({'a':[]})          # ValueError: empty, can't determine element type
    await kv.set({'a':[1, 2, 3]})   # Ok
    await kv.set({'b':[1, 2.5, 3]}) # ValueError: int, float, int    
    ```

```py
from fc.kv import KV

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
```

```
Age: 25
Player: Monster, Active: True
['Armour', 'Kilt']
```

## Exceptions

- `ResponseError` : raised when a failed response is received
- `ValueError` : raised by the API before a request is sent
- `OSError` : if opening the connection fails


## Types

|Python|FlatBuffers/FlexBuffers|Server|
|---|---|---|
|int|int|`int64`|
|float|float|`float` (4 bytes)|
|str|string|`std::vector<char>`|
|bool|bool|`bool`|
|list[int]|TypedVector: VECTOR_INT|`std::vector<int64>`|
|list[float]|TypedVector: VECTOR_FLOAT|`std::vector<float>`|
|list[str]|TypedVector: VECTOR_KEY|`std::vector<std::string>`|
|list[bool]|TypedVector: VECTOR_BOOL|`std::vector<bool>`|

!!! note
    A `TypedVector` is a vector (contigious array) where each element
    is the same type.


