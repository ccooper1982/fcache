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


## Exceptions

- `ResponseError` : raised when a failed response is received
- `ValueError` : raised by the API before a request is sent
- `OSError` : if opening the connection fails


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


```bash title='Output'
Age: 25
Player: Monster, Active: True
['Armour', 'Kilt']
```

### Types

|Python|FlatBuffers/FlexBuffers|Server|
|---|---|---|
|int|int|`int64`|
|float|float|`float` (4 bytes)|
|str|string|`std::vector<char>`|
|bool|bool|`bool`|
|bytes|BLOB|`std::vector<uint8_t>`|
|list[int]|TypedVector: VECTOR_INT|`std::vector<int64>`|
|list[float]|TypedVector: VECTOR_FLOAT|`std::vector<float>`|
|list[str]|TypedVector: VECTOR_KEY|`std::vector<uint8_t>`|
|list[bool]|TypedVector: VECTOR_BOOL|`std::vector<bool>`|

!!! note
    A `TypedVector` is a type of FlexBuffer vector, where each element is the same type.
    



## Lists

Lists are node based doubly linked lists. It's not yet possible to sort the list or create a sorted list.

```py
from fc.list import List

async def lists():
  if (client := await connect()) is None:
    return
  
  # create API object for list functions
  list = List(client)

  await list.delete_all()

  # create list for integers
  await list.create('scores', type='int')
  # add these items to head
  await list.add_head('scores', [25,35,45,55])
  # insert in between 35 and 45 (at position 2)
  await list.add('scores', [40], pos=2)
  # add two more to the tail
  await list.add_tail('scores', [60, 65])
  
  # get everything from the first item
  print(f"a. {await list.get_n('scores')}")
  # get everything in reverse
  print(f"b. {await list.get_n_reverse('scores')}")
  # # get the first 3 
  print(f"c. {await list.get_n('scores', count=3)}")
  # # get the last 2
  print(f"d. {await list.get_n('scores', start=5)}")
  # get middle 3 with range (could also use get_n())
  print(f"e. {await list.get_range('scores', start=2, stop=5)}")
  # get middle 5 in reverse, using negative index
  print(f"f. {await list.get_range_reverse('scores', start=1, stop=-1)}")


  # list of strings
  await list.create('names', type='str')
  
  await list.add_head('names', ['Adam','Charlie'])
  print(await list.get_n('names'))

  await list.add('names', ['Bob'], pos=1)
  print(await list.get_n('names'))
```

```bash title='Output'
a. [25, 35, 40, 45, 55, 60, 65]
b. [65, 60, 55, 45, 40, 35, 25]
c. [25, 35, 40]
d. [60, 65]
e. [40, 45, 55]
f. [60, 55, 45, 40, 35]
['Adam', 'Charlie']
['Adam', 'Bob', 'Charlie']
```