# remove

```py
async def remove(self, name:str, *, start: int = 0, stop: int = None) -> None
```
Remove items from a list using the range: `[start, stop)`.

`start` and `stop` use the same rules as [get_range()](./get_range.md), so negative
and positive values are permitted, but they must translate to positions where `start < stop`.


## Examples

```py
await list.create('rmv', type='int')

await list.add_head('rmv', [0,1,2,3,4,5,6,7,8,9,10])
print(await list.get_n('rmv'))

await list.remove('rmv', start=0, stop=3)
print(await list.get_n('rmv'))

await list.remove('rmv', start=1, stop=-1)
print(await list.get_n('rmv'))

await list.remove('rmv')  # remove all
print(await list.get_n('rmv'))
```

```bash title='Output'
[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
[3, 4, 5, 6, 7, 8, 9, 10]
[3, 10]
[]
```