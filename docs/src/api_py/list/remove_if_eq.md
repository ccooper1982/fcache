# remove_if_eq

```py
async def remove_if_eq(name:str, *, start: int = 0, stop: int = None, val):
```
Search items in range: `[start, stop)`, removing those with value equal to `val`.

`start` and `stop` use the same rules as [get_range()](./get_range.md), so negative
and positive values are permitted, but they must translate to positions where `start < stop`.

!!! note 
    No gaurantees on behaviour if `val` is not the same type as when the list was created.
    This will be addressed in a future release.


## Examples

```py
await list.create('rmv_if', type='int')

await list.add_head('rmv_if', [0,1,2,5,5,5,6,7,8,9,7,7,10])
print(await list.get_n('rmv_if'))

await list.remove_if_eq('rmv_if', start=0, stop=7, val=5)
print(await list.get_n('rmv_if'))

await list.remove_if_eq('rmv_if', start=-3, val=7)
print(await list.get_n('rmv_if'))
```

```bash title='Output'
[0, 1, 2, 5, 5, 5, 6, 7, 8, 9, 7, 7, 10]
[0, 1, 2, 6, 7, 8, 9, 7, 7, 10]
[0, 1, 2, 6, 7, 8, 9, 10]
```