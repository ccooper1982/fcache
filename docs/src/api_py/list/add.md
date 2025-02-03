# add

__Unsorted List__
```py
async def add(name: str,
              items: List[int|str|float], *,
              pos:int = 0) -> int
```

__Sorted List__
```py
async def add(name: str,
              items: List[int|str|float],
              items_sorted:bool = False) -> int
```

Both functions insert items but `pos` cannot be defined for a sorted list.

All elements in `items` must be the same type as when `list` was created.


## Unsorted
|Param|Info|
|:---|:---|
|pos|The position to begin inserting|

Inserts items into the list, starting at position `pos`, in the same order as `items`.

- If `abs(pos)` is greater than the list size, the items are appended


## Sorted
|Param|Info|
|:---|:---|
|items_sorted|`True`: set only if `items` are already sorted in __ascending__ order<br/>`False`: the server handles sorting|


## Returns
Both functions return the length of the list after adding `items`.


## Examples

```py title='Unsorted'
# lst is a fc.list.UnsortedList

await lst.create('names', type='str')

await lst.add('names', ['Arya', 'Fiona']) # same as add_head()
print(await lst.get_n('names'))

await lst.add('names', ['David', 'Bob', 'Charlie'], pos=1)
print(await lst.get_n('names'))

finalSize = await lst.add('names', ['Emma'], pos=3)
print(f'Final size: {finalSize}')
print(await lst.get_n('names'))
```

```bash title='Output'
['Arya', 'Fiona']
['Arya', 'David', 'Bob', 'Charlie', 'Fiona']
['Arya', 'David', 'Bob', 'Emma', 'Charlie', 'Fiona']
Final size: 6
```


```py title='Sorted'
# lst is a fc.list.SortedList

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