# get

```py
async def get(key=None, keys=[]) -> dict | Any
```

Gets a single key or multiple keys.

## Returns
Called with:

- `key`: the value is returned
- `keys`: all key values returned in a `dict`


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