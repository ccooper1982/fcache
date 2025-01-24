# create

```py
async def create(*, name: str, type: str, failOnDuplicate=True) -> None
```

Creates a new list with the given name and type.

|Param|Info|
|:---|:---|
|name|Name of the list, used in subsequent list functions|
|type|Must be one of: `int` or `uint`|
|failOnDuplicate|`True`: a ResponseError will raise if a list with this name already exists<br/>`False`: and this name already exists, no changes are made to the list and a ResponseError is not raised|

## Examples

```py
# create a list called 'scores'
await list.create(name='scores', type='int')

# add four items to the head
await list.add_head('scores', items=[1,2,3,4])

# add item to tail
await list.add_tail('scores', items=[5])
```