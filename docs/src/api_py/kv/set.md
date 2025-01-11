# set

```py
async def set(kv: dict) -> None
```

Sets new key(s). If a key already exists, the value is replaced.

!!! note
    If value is a `list`, all items must be the same type.


## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True})
```