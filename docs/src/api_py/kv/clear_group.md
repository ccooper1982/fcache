# clear_group

```py
async def clear_group(name:str, delete_group:bool = True) -> None:
```

Deletes all keys in a group, and optionally will also delete the group.


## Examples

```py
await kv.set({'user':'user1', 'age':25}, group='g1')

await kv.clear_group('g1', False) # group still exists, but has no keys

await kv.clear_group('g1') # now group is deleted

```