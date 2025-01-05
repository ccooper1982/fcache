# set

```py
async def set(kv: dict) -> None
```

Sets new key(s). If a key already exists, the value is replaced.

!!! warning
    The value of a key cannot be an iterable. This will be added in a future release.


## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True})
```