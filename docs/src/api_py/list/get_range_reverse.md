# get_range_reverse

```py
async def get_range_reverse(name: str, *, start:int, stop: int = None) -> list:
```
Get items in range `[start, stop)`, but iterates from __tail to head__.

This is the same as [get_range()](./get_range.md) but positions are relative to the tail, i.e.:

- `start = 0` is the tail node
- `start = -1` is the head node


If `stop` is `None`, returns all remaining.

Either or both `start` and `stop` can be negative/positive, but they must represent indices such that `start < stop`.

## Returns
A list of node values. The list will be empty if:

- the range is invalid
- `start == stop`

## Examples

```py
await list.create('list', type='str')
await list.add_head('list', ['A', 'B', 'C', 'D', 'E'])

print(await list.get_range_reverse('list', start=0))    # 0 is the tail
print(await list.get_range_reverse('list', start=2))
print(await list.get_range_reverse('list', start=-2))
print(await list.get_range_reverse('list', start=1, stop=-2))
print(await list.get_range_reverse('list', start=-4, stop=-1))
```

```bash title='Output'
['E', 'D', 'C', 'B', 'A']
['C', 'B', 'A']
['B', 'A']
['D', 'C']
['D', 'C', 'B']
```