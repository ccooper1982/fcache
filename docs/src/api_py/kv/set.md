# set

```py
async def set(kv:dict, group:str = None) -> None
```

Sets new key(s). If a key already exists, the value is replaced.

- `kv` : the key-values, where a value can be: int, str, float, bool or a list of those.
- `group` : a group name into which the keys will be stored

The `group` is created if it does not exist.


!!! note
    If a value is a list, all elements in the list must be the same type.

## Examples

```py title='Basics'
await kv.set({'user':'user1', 'age':25, 'active':True})
```

```py title='Blob'
file_data = bytes()
with open('cat.jpg', mode='rb') as file:
  file_data = file.read()

kv = KV(client)

await kv.set({'img_cat':file_data})

if data := await kv.get(key='img_cat'):
  with open('cat_rsp.jpg', mode='wb') as file:
    file.write(data)
    print('Image written')
```


```py title='Groups'
await kv.set({'username':'user1', 'city':'London'}, group='a@xyz.com')
await kv.set({'username':'user2', 'city':'Paris'}, group='b@xyz.com')

print(await kv.get(key='username', group='a@xyz.com'))
print(await kv.get(key='username', group='b@xyz.com'))

print(await kv.get(keys=['username', 'city'], group='a@xyz.com'))
print(await kv.get(keys=['username', 'city'], group='b@xyz.com'))
```

```sh title='Output'
user1
user2
{'city': 'London', 'username': 'user1'}
{'city': 'Paris', 'username': 'user2'}
```