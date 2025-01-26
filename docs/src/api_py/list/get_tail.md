# get_tail

```py
async def get_tail(name: str):
```

Returns the item at the tail.

## Returns
If the list is empty returns `None`, otherwise the item.

## Examples

```py
await list.create('names', type='str')
await list.add_head('names', ['Arya', 'Fiona'])
print(await list.get_tail('names')) # 'Fiona'
```