# set

```py
async def set(name:str, items: typing.List[int|str|float], *, pos:int=0):
```

Replace list node values, starting at `pos`, with `items`. The list items are overwritten
in the same order as `items`.

- All `items` must be the same type 
- The type must be the same as when the list was created

Items are overwritten until either the end of items or end of list.


## Examples

```py
await list.create('set', type='str')

await list.add_head('set', ['A','B', 'X', 'Y', 'Z','F','G'])
print(await list.get_n('set'))

# replace 'X'
await list.set('set', ['C'], pos=2)
print(await list.get_n('set'))

# replace 'Y','Z'
await list.set('set', ['D','E'], pos=3)
print(await list.get_n('set'))

# len(items) 4, but pos=5 leaves only 2 items, so last two
# list nodes are replaced, no error
await list.set('set', ['A','D','A','D'], pos=5)
print(await list.get_n('set'))
```

```bash title='Output'
['A', 'B', 'X', 'Y', 'Z', 'F', 'G']
['A', 'B', 'C', 'Y', 'Z', 'F', 'G']
['A', 'B', 'C', 'D', 'E', 'F', 'G']
['A', 'B', 'C', 'D', 'E', 'A', 'D']
```
