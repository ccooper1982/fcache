# create

```py
async def create(name: str, *, type: str, failOnDuplicate=True) -> None
```

Creates a new list with the given name and type.

|Param|Info|
|:---|:---|
|name|Name of the list, used in subsequent list functions|
|type|Must be one of: `int`, `float` or `str`|
|failOnDuplicate|`True`: a ResponseError is raised if a list with this name already exists<br/>`False`: if the name already exists, no changes are made to the list and a ResponseError is not raised|

## Examples

```py
# create a list called 'scores'
await list.create('scores', type='int')
# add four items to the head
await list.add_head('scores', [1,2,3,4])

await list.create('names', type='str')
await list.add_head('names', ['Alice', 'Charlie'])
await list.add('names', items=['Bob'], pos=1)
```