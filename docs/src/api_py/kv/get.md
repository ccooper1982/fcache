# get

```py
async def get(*,  key:str = None,
                  keys:List[str] = [],
                  group:str = None) -> dict | Any
```

Gets a single key or multiple keys.

There are alternative convenience functions:

- [get_key](./get_key.md) - get a single key
- [get_keys](./get_keys.md) - get multiple keys
- [get_all](./get_all.md) - get all keys in a group


## Returns
Called with:

- `key`: if key exists the value is returned, otherwise `None`
- `keys`: all key values in a `dict`. If no keys exist, an empty `dict` is returned


## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True})

# get single key, returns the value
age = await kv.get(key='age')
print(f'Age: {age}')

# get multiple keys, returns dict
rsp = await kv.get(keys=['user', 'active'])
print(f"User: {rsp['user']}, Active: {rsp['active']}")
```

```
Age: 25
User: user1, Active: True
```