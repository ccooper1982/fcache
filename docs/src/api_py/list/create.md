# create

```py
async def create(name: str, *, type: str, fail_on_duplicate:bool = True) -> None
```

Creates a new list with the given name and type.

|Param|Info|
|:---|:---|
|name|Name of the list, used in subsequent list functions|
|type|Must be one of: `int`, `float` or `str`|
|fail_on_duplicate|`True`: `ResponseError` is raised if a list with this `name` already exists<br/>`False`: if the name already exists, no changes are made to the existing list and an `ResponseError` is not raised|

## Examples

```py
# create an int list called 'scores' and add four items
await list.create('scores', type='int')
await list.add_head('scores', [1,2,3,4])

# string list
await list.create('names', type='str')
await list.add_head('names', ['Alice', 'Charlie'])
await list.add('names', ['Bob'], pos=1)
await list.add_tail('names', ['David'])
# [Alice, Bob, Charlie, David]
```