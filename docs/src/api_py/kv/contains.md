# contains

```py
async def contains(keys: list, *, group: str = None) -> set
```

Given a list of keys, return those which exist.


## Returns
A `set` of keys that exist. If no keys exist, an empty set is returned.


## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True, 'city':'London'})
await kv.contains(['age','user','dont_exist']) # returns {'age','user'}
```