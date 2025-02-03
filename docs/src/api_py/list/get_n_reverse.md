# get_n_reverse

```py
async def get_n_reverse(name: str, *, start: int = 0, count: int = None) -> list:
```

Returns `count` number of items, beginning from `start` position, iterating from __tail to head__.

This is the same as [get_n()](./get_n.md) but`start` is relative to the tail: i.e. `start = 0` is the tail.

- `count`
    - `None` will get to and including the head
    - Cannot be negative

- `start` cannot be negative


## Examples

```py
await lst.create('chars', type='str')
await lst.add('chars', ['A', 'B', 'C', 'D', 'E'])

print(await lst.get_n_reverse('chars'))           
print(await lst.get_n_reverse('chars', start=3))  
print(await lst.get_n_reverse('chars', start=1, count=2)) 
print(await lst.get_n_reverse('chars', count=3))          
```

```bash title='Output'
['E', 'D', 'C', 'B', 'A']
['B', 'A']
['D', 'C']
['E', 'D', 'C']
```