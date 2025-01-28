# get_n

```py
async def get_n(name: str, *, start: int = 0, count: int = None) -> list:
```

Returns `count` number of items, beginning from `start` position.

- `count`
    - `None` will get until to and including the tail
    - Cannot be negative

- `start` cannot be negative


!!! note
    `start` is zero based. The head is `0`.


## Examples

```py
await list.create('list', type='str')
await list.add_head('list', ['A', 'B', 'C', 'D', 'E'])

print(await list.get_n('list'))           # all from head
print(await list.get_n('list', start=3))  # all from 4th item
print(await list.get_n('list', start=1, count=2)) # 2 from 2nd item
print(await list.get_n('list', count=3))          # 3 from head
```

```bash title='Output'
['A', 'B', 'C', 'D', 'E']
['D', 'E']
['B', 'C']
['A', 'B', 'C']
```