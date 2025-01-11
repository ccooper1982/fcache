# add

```py
async def add(kv: dict) -> None
```

Sets new key(s) but unlike [set](./set.md), if a key already exists, the value is __not__ replaced.

!!! note
    If value is a `list`, all items must be the same type.

## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True})
# calling add() will not update age
await kv.add({'age':25})
```