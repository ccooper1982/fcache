# Key Values



## List/Array/Vector Values
A value which is not scalar is serialised as a `TypedVector`. This means that all elements of the vector must be the same type. 

Using Python as an example:

```py title='Lists'
data =
{
  'animals':['cat', 'dog', 'horse'],
  'scores':[50,55,5]
  'anything':['xyz', 15, False]
}
```

Storing `animals` and `scores` will succeed but `anything` will fail.


## Caching JSON
There is no native JSON support, so JSON must be stored as either as a string or converting to a compatible dictionary/map structure.

!!! note
    The Python API functions `set`, `add` and `clear_set` accept `dict` for key values.