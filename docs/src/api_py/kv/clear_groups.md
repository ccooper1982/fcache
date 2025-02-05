# clear_groups

```py
async def clear_groups(delete_groups:bool = True) -> None:
```

Delete all groups or only delete keys in all groups.

If `delete_groups` is `True`, all groups deleted. Otherwise, all groups have keys deleted, but the groups still exist.


## Examples

```py
await kv.set({'user':'user1', 'age':25}, group='g1')
await kv.set({'user':'user2', 'age':45}, group='g2')

await kv.clear_groups(False) # g1 and g2 still exist, but have no keys
await kv.clear_groups() # all groups deleted

```