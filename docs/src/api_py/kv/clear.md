# clear

```py
async def clear() -> None
```

Deletes all keys.

You can also use [`clear_set()`](./clear_set.md) to delete all keys then set new keys in a single call.

## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True, 'city':'London'})
await kv.clear()
```