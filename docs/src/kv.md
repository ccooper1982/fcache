# Key Values

- Key must be a string
- Supported values:    
    - integer
    - unsigned integer
    - float
    - bool
    - string
    - List/vector/array of the above


## List/Array/Vector Values
A value which is not scalar is serialised as a `TypedVector`. This means that all elements of the vector must be the same type. 

Using Python as an example:

```py title='Lists'
data =
{
  'animals':['cat', 'dog', 'horse'],
  'scores':[50,55,5],
  'anything':['xyz', 15, False]
}
```

Storing `animals` and `scores` will succeed but `anything` will fail.


## Groups
Groups is a simple solution to separating related keys. Rather than using delimiters in keys such as `user1:name` and `user1:age`, you create a group for `user1` then store the `name` and `age` in that group.

The group is a string that unique identifies an entity. For example for a user that could be an email address or user ID, or for hardware it could be a device ID or a MAC address.


## Caching JSON
There is no native JSON support, so JSON must be stored as either as a string or converting to a compatible dictionary/map structure.

!!! note
    The Python API functions `set`, `add` and `clear_set` accept `dict` for key values.