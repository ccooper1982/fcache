# clear_set

```py
async def clear_set(kv:dict) -> None:
```

Deletes all keys then sets new keys in a single command.


!!! warning
    The value of a key cannot be an iterable. This will be added in a future release.


## Examples

```py
await kv.set({'k1':10, 'k2':20})
await kv.clear_set({'k3':30, 'k4':40})
# only k3 and k4 exist
```