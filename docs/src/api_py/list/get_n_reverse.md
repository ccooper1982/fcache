# get_n_reverse

```py
async def get_n_reverse(name: str, *, start: int = 0, count: int = 0) -> list:
```

Returns `count` number of items, beginning from `start` position.

This is the same as [get_n()](./get_n.md) but `start` is from the tail, i.e. `start=0` is the tail,
rather than the head.

- `count`
    - Value `0` means get to the head
    - Cannot be negative

- `start` cannot be negative


!!! note
    `start` is zero based. The __tail__ is `0`.


## Examples

```py
await list.create('list', type='str')
await list.add_head('list', ['A', 'B', 'C', 'D', 'E'])

print(await list.get_n_reverse('list'))           # all from tail
print(await list.get_n_reverse('list', start=3))  # all from 4th item
print(await list.get_n_reverse('list', start=1, count=2)) # 2 from 2nd item
print(await list.get_n_reverse('list', count=3))          # 3 from tail
```

```bash title='Output'
['E', 'D', 'C', 'B', 'A']
['B', 'A']
['D', 'C']
['E', 'D', 'C']
```