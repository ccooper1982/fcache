# clear_set

```py
async def clear_set(kv:dict) -> None:
```

Deletes all keys then sets new keys in a single command.


!!! note
    If value is a `list`, all items must be the same type.


## Examples

```py
await kv.set({'k1':10, 'k2':20})
await kv.clear_set({'k3':30, 'k4':40})
# only k3 and k4 exist
```