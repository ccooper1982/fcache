# get_range_reverse

```py
async def get_range_reverse(name: str, *, start:int, stop: int = None) -> list:
```
Returns items based on a range defined by `start` and `stop` positions.

This is the same as [get_range()](./get_range.md) but:

- positive indices are relative to the tail
    - `start=0` is the tail    
- negative indices are relative to the head
    - `stop=-1` is the head



The range is `[start, stop)`, i.e. `stop` is exclusive.

If `stop` is `None`, returns all remaining.

`start` and `stop` can be negative, similar to Python slicing:

- `-1` is the head
- `-2` is the second node (the node after the head)

Either or both `start` and `stop` can be negative/positive, but they must represent indices such that `start < stop`.

## Returns
A list of node values, or an empty list if the range is invalid of if `start == stop`.

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