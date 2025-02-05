# clear_set

```py
async def clear_set(kv:dict, group:str = None) -> None:
```

Deletes all keys then sets new keys in a single command.

This command may be more convenient or efficient than using [set](./set.md).

This is useful if there are many keys to update and the full set of keys are available. This is convenient to
add new keys, update existing keys and delete keys that are no longer required.

## Examples

```py
await kv.set({'k1':10, 'k2':20})
await kv.clear_set({'k3':30, 'k4':40})
# only k3 and k4 exist
```

```py title='With Group'
await kv.set({   
                'username':'some_username',
                'city':'London',
                'status':'Active',
                'client_version':2
              },
              group='a@xyz.com')

# update existing keys (username, status), set a new key (age) and
# delete unused key (client_version)
await kv.clear_set({
                      'username':'new_username',
                      'city':'London',
                      'age':55,
                      'status':'AuthRequired'
                   },
                   group='a@xyz.com')
```