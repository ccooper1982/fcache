# get_key

```py
async def get_key(key:str, group:str = None) -> Any
```

Get a key.

If `group` is set, only that group is checked.


## Returns
- The key's value
- If the key does not exist, `None` is returned.


## Examples


```py
await kv.set({'k1':'key1_value', 'k2':'key2_value'})
print(await kv.get_key('k2'))
```

```sh title='Output'
key2_value
```

```py
await kv.set({'username':'user1', 'city':'London'}, group='a@xyz.com')
print(await kv.get_key('city', 'a@xyz.com'))
```

```sh title='Output'
London
```