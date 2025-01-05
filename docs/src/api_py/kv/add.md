# add

```py
async def add(kv: dict) -> None
```

Sets new key(s) but unlike [set](./set.md), if a key already exists, the value is __not__ replaced.

!!! warning
    The value of a key cannot be an iterable. This will be added in a future release.


## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True})
# calling add() will not update age
await kv.add({'age':25})
```