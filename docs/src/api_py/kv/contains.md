# contains

```py
async def contains(keys: list) -> set
```

Given a list of keys, return those which exist.


## Returns
A `set` of keys that exist.


## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True, 'city':'London'})
exist = await kv.count()
# exist contains 'age' and 'user'
```