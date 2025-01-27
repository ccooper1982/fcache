# add

__Unsorted List__
```py
async def add(name: str, items: typing.List[int|str|float], *, pos: int) -> None
```

__Sorted List__
```py
async def add(name: str, items: typing.List[int|str|float], items_sorted:bool = False) -> None
```

These functions add items to the list. The only difference being that a sorted list determines the order so the `pos` is not
relevant.

|Param|Info|
|:---|:---|
|name|Name of the list|
|items|The items to add to the list. Each item must be of the appropriate type for the list.|


## Unsorted
|Param|Info|
|:---|:---|
|pos|The position to begin inserting. Positions begin at `0`.|

Inserts items into the list, starting at position `pos`, in the same order as `items`.

- `pos` must be `>= 0`
- If `pos` is higher than the list size, the items are appended




## Sorted
|Param|Info|
|:---|:---|
|items_sorted|`True`: set only if `items` are already sorted in __ascending__ order<br/>`False`: the server will sort during insertion|

The sort order is determined by the `<` operator and is always ascending order.



## Examples

```py title='Unsorted'
from fc.list import UnsortedList

lst = UnsortedList()

await lst.create('names', type='str')

await lst.add('names', ['Arya', 'Fiona'], pos=0) # alternative to add_head()
print(await lst.get_n('names'))

await lst.add('names', ['David', 'Bob', 'Charlie'], pos=1)
print(await lst.get_n('names'))

await lst.add('names', ['Emma'], pos=3)
print(await lst.get_n('names'))
```

```bash title='Output'
['Arya', 'Fiona']
['Arya', 'David', 'Bob', 'Charlie', 'Fiona']
['Arya', 'David', 'Bob', 'Emma', 'Charlie', 'Fiona']
```


```py title='Sorted'
from fc.list import SortedList

lst = SortedList()

await lst.create('names', type='str')

await lst.add('names', ['Arya', 'Fiona'], items_sorted=True)
print(await lst.get_n('names'))

await lst.add('names', ['David', 'Bob', 'Charlie'])
print(await lst.get_n('names'))

await lst.add('names', ['Emma'])
print(await lst.get_n('names'))
```

```bash title='Output'
['Arya', 'Fiona']
['Arya', 'Bob', 'Charlie', 'David', 'Fiona']
['Arya', 'Bob', 'Charlie', 'David', 'Emma', 'Fiona']
```