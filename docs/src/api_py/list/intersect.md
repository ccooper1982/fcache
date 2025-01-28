# intersect

```py
async def intersect(list1: str, list2: str, *,
                    l1_start:int=0, l1_stop:int=None,
                    l2_start:int=0, l2_stop:int=None) -> typing.List[int|float|str]:
```

Intersects `list1` and `list2`. Positions within each list can be defined, with defaults applying the intersection over each entire list.

- `l1_start` and `l1_stop` restrict the intersection in `list1` to `[l1_start, l1_stop)`
- `l2_start` and `l2_stop` restrict the intersection in `list2` to `[l2_start, l2_stop)`

In both cases, the stop positions are exclusive.


## Examples

The code comments below exclude the value at the stop position as it's exclusive.

```py title='Int Lists'
await lst.create('i1', type='int')
await lst.create('i2', type='int')

await lst.add('i1', [0,1,2,5,5,5,6,7,8,9,10], items_sorted=True)
await lst.add('i2', [0,1,2,5,5,5,6,7], items_sorted=True)

# entire lists
print(await lst.intersect('i1', 'i2'))

# [0,1,2,5,5,5] n [0,1,2,5,5,5,6,7]
print(await lst.intersect('i1', 'i2', l1_stop=6))

# [0,1,2,5,5,5,6,7,8,9,10] n [0,1,2]
print(await lst.intersect('i1', 'i2', l2_stop=3))

# [5,5,5] n [0,1,2,5,5,5]
print(await lst.intersect('i1', 'i2', l1_start=3, l1_stop=6, l2_stop=-2))
```

```sh title='Output'
[0, 1, 2, 5, 5, 5, 6, 7]
[0, 1, 2, 5, 5, 5]
[0, 1, 2]
[5, 5, 5]
```

<br/>

```py title='String Lists'
await lst.create('s1', type='str')
await lst.create('s2', type='str')

await lst.add('s1', ['apple', 'cider', 'painful', 'tequila', 'yes'], items_sorted=True)
await lst.add('s2', ['apple', 'beer', 'cider', 'no', 'painful', 'tequila'], items_sorted=True)

# entire lists
print(await lst.intersect('s1', 's2'))

# ['apple', 'cider'] n ['apple', 'beer', 'cider']
print(await lst.intersect('s1', 's2', l1_stop=2, l2_stop=-3))

# ['painful', 'tequila', 'yes'] n ['painful', 'tequila']
print(await lst.intersect('s1', 's2', l1_start=2, l2_start=4))
```

```sh title='Output'
['apple', 'cider', 'painful', 'tequila']
['apple', 'cider']
['painful', 'tequila']
```