# add

```py
async def add(name: str, items: typing.List[int], *, pos: int) -> None
```

Inserts items to a list, starting at position `pos`. Items are added in the same order as `items`.

!!! note
    `pos` is zero-based, the first node in the list is position `0`.


|Param|Info|
|:---|:---|
|name|Name of the list|
|items|The items to add to the list. Each item must be of the appropriate type for the list.|
|pos|The position to begin inserting. Positions begin at `0`.|

## Examples

```py
await list.create('names', type='str')
# alternative to add_head()
await list.add('names', ['Arya', 'Fiona'], pos=0)
# [Arya, Fiona]
await list.add('names', ['David', 'Charlie'], pos=1)
# [Arya, Charlie, David, Fiona]
await list.add('names', ['Emma'], pos=3)
# [Arya, Charlie, David, Emma, Fiona]
```