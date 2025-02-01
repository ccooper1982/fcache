# Lists

A list is a node based doubly linked list with support for:

- integers (signed and unsigned)
- floats
- strings


Lists can be sorted or unsorted: 

- An unsorted list retains the order as added
- A sorted list ensures nodes are always in ascending order. If the items being added are already sorted, this can be signalled when adding, reducing workload 

For both sorted and unsorted lists get items:

- Based on a range `[start,stop)` with negative positions permitted (similar to Python slicing)
- Some commands have reverse versions which iterate the list from tail to head


## Add Items

- Unsorted List: items can be added anywhere in the list
- Sorted List: the list is sorted automatically, in ascending order


## Get Items
The server API allows iterating a list from head to tail or tail to head.

In the Python API there is `get_n()` which returns `n` items from a given starting position.

```py
await lists.get_n('scores', start=0, count=3)
```

This returns the first three items.

The reverse version iterates from the tail to head, so returns the last three items:

```py
await lists.get_n_reverse('scores', start=0, count=3)
```

## Remove Items
Items can be removed using their position or based on a value.

For example, in the Python API we can remove nodes with a value of `100` in range `[0,8)`:

```py
await lists.remove_if_eq('scores', start=0, stop=8, val=100)
```


## Intersect
Sorted lists of the same type can be intersected, with support for start/stop positions within each list.


```py
await lst.create('i1', type='int')
await lst.create('i2', type='int')

await lst.add('i1', [0,1,2,5,5,5,6,7,8,9,10], items_sorted=True)
await lst.add('i2', [0,1,2,5,5,5,6,7], items_sorted=True)

print(await lst.intersect('i1', 'i2'))
print(await lst.intersect('i1', 'i2', l1_stop=6))

# you can also set new_name_list, which creates a new list for
# for the intersected items, rather than returning them
```

```sh title='Output'
[0, 1, 2, 5, 5, 5, 6, 7]
[0, 1, 2, 5, 5, 5]
```