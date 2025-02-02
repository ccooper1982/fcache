# add_tail

```py
async def add_tail(name: str, items: typing.List[int|str|float]) -> None
```
Appends items to the list. Items are added in the same order as `items`.

|Param|Info|
|:---|:---|
|name|Name of the list|
|items|The items to add to the list. Each item must be of the appropriate type for the list.|

## Examples

```py
await list.create('names', type='str')
# you can add_tail() on an empty list
await list.add_tail('names', ['Arya', 'Charlie'])
# [Arya, Charlie] , Arya is the head, Charlie the tail
await list.add_tail('names', ['David', 'Fiona'])
# [Arya, Charlie, David, Fiona]
```