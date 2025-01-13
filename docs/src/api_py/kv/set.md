# set

```py
async def set(kv: dict) -> None
```

Sets new key(s). If a key already exists, the value is replaced.

!!! note
    If value is a `list`, all items must be the same type.


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