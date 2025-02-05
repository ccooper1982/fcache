# count

```py
async def count(*, group: str = None) -> int
```

Gets the number of keys.

If `group` is set, the count only applies to that group.

## Returns
The number of keys that exist. If `group` is set but does not exist, `0` is returned.


## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True, 'city':'London'})
count = await kv.count() # 4
```