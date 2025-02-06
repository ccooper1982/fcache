# get_keys

```py
async def get_keys(keys:List[str], group:str = None) -> dict
```

Get multiple keys.

If `group` is set, only that group is checked.

## Returns
- A `dict` of the key-values
- If none of the keys exist, an empty `dict`

## Examples

```py
await kv.set({'username':'user1', 'city':'London', 'age':25}, group='a@xyz.com')
print(await kv.get_keys(['city','age'], 'a@xyz.com'))
```

```sh title='Output'
{'city':'London', 'age':25}
```