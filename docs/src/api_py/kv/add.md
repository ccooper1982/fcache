# add

```py
async def add(kv: dict, group: str = None) -> None
```

The same as [set](./set.md) except if a key already exists, the value is __not__ replaced.


## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True})
# calling add() will not update age
await kv.add({'age':35})
```