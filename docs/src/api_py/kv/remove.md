# remove

```py
async def remove(*, key:str = None,
                    keys:List[str] = list(),
                    group: str = None) -> None:
```

Deletes one or multiple keys.

- `key` : individual key to delete
- `keys` : list of keys to delete

If a key does not exist it is not considered an error.


## Examples

```py
await kv.set({'user':'user1', 'age':25, 'active':True, 'city':'London'})
await kv.remove(key='age')
await kv.remove(keys=['user','city'])
# only 'active' remains
```