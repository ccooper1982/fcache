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
- Some commands have reverse versions, which work similar to C++'s reverse iterators


## Add Items

- Unsorted List: items can be added anywhere in the list
- Sorted List: order is defined by the sorted order


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
