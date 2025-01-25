# get_head

```py
async def get_head(name: str):
```

Returns the item at the head.

## Returns
If the list is empty returns `None`, otherwise the item.

## Examples

```py
await list.create('names', type='str')
await list.add_head('names', ['Arya', 'Fiona'])
print(await list.get_head('names')) # 'Arya'
```