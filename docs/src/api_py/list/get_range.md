# get_range

```py
async def get_range(name: str, *, start:int, stop: int = None) -> list:
```

Get items using the range: `[start, stop)`.

If `stop` is `None`, returns all remaining.

`start` and `stop` can be negative, similar to Python slicing:

- `-1` is the tail
- `-2` is the node before the tail


Either or both `start` and `stop` can be negative/positive, but they must represent indices such that `start < stop`.

## Returns
A list of node values. The list will be empty if:

- the range is invalid
- `start == stop`

## Examples

```py
await list.create('list', type='str')
await list.add_head('list', ['A', 'B', 'C', 'D', 'E'])

print(await list.get_range('list', start=0))
print(await list.get_range('list', start=2))
print(await list.get_range('list', start=-2))
print(await list.get_range('list', start=1, stop=-2))
print(await list.get_range('list', start=-4, stop=-1))

# invalid: [3,1)
print(await list.get_range('list', start=-2, stop=1))
# invalid: [3,2)
print(await list.get_range('list', start=3, stop=-3))
```

```bash title='Output'
['A', 'B', 'C', 'D', 'E']
['C', 'D', 'E']
['D', 'E']
['B', 'C']
['B', 'C', 'D']
[]
[]
```

## Notes
The API performs simple checks on the ranges to spot problems before sending the query, but it's not
always possible without knowing the list's size.

Consider the example above, this range is invalid:

```py
print(await list.get_range('list', start=3, stop=-3))
```

But add two items and try again:

```py
await list.add_tail('list', ['F','G'])
# now [3,4)
print(await list.get_range('list', start=3, stop=-3)) 
```

```bash title='Output'
['D']
```
