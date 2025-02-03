# add_head

```py
async def add_head(name: str, items: typing.List[int|str|float]) -> int
```
Adds to items to the list's head. Items are added in the same order as `items`.

|Param|Info|
|:---|:---|
|name|Name of the list|
|items|The items to add to the list. Each item must be of the appropriate type for the list.|

## Returns
The length of the list after adding `items`.

## Examples

```py
await list.create('names', type='str')
await list.add_head('names', ['Arya', 'Charlie'])
# [Arya, Charlie]
await list.add_head('names', ['Adam', 'Alice'])
# [Adam, Alice, Arya, Charlie]
```