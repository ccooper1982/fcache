# count

```py
async def count() -> int
```

Gets the number of keys.


## Returns
The number of keys that exist.


## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True, 'city':'London'})
count = await kv.count() # 4
```