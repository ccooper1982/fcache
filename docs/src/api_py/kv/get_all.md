# get_all

```py
async def get(group:str) -> dict
```

Gets all keys in the group.


## Examples

```py
await kv.set({'username':'user1', 'city':'London'}, group='a@xyz.com')
print(await kv.get_all('a@xyz.com'))
```

```sh title='Output'
{'city': 'London', 'username': 'user1'}
```