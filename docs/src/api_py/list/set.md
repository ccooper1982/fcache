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
await list.create('chars', type='str')
await list.add_head('chars', ['A','B','X','Y','Z','F','G'])
print(await list.get_n('chars'))

# replace 'X'
await list.set('chars', ['C'], pos=2)
print(await list.get_n('chars'))

# replace 'Y','Z'
await list.set('chars', ['D','E'], pos=3)
print(await list.get_n('chars'))

# replace last four
await list.set('chars', ['H','E', 'L','O'], pos=-4)
print(await list.get_n('chars'))

```

```bash title='Output'
['A', 'B', 'X', 'Y', 'Z', 'F', 'G']
['A', 'B', 'C', 'Y', 'Z', 'F', 'G']
['A', 'B', 'C', 'D', 'E', 'F', 'G']
['A', 'B', 'C', 'H', 'E', 'L', 'O']
```
