# info

```py
async def info (name:str) -> typing.Tuple[int,str,bool]:
```

Returns list information.

## Returns
Tuple: `(list_size, list_type, list_sorted)`

- `list_size` is the number of items in the list, and is equal to [size()](./size.md)
- `list_type` is one of: `str`, `int` or `float`
- `list_sorted` is `True` if the list is a sorted list (created with `fc.list.SortedList`), otherwise `False`

