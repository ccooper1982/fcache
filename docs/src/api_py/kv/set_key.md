# set_key

```py
async def set_key(key:str, value:Any, group:str = None) -> None
```

Sets a new key. If the key already exists, the value is replaced.

- `value` : can be `int`, `str`, `float`, `bool` or a `list` of those
- `group` : a group name in which the key will be stored

The `group` is created if it does not exist.

You can set multiple keys using [set](./set.md).

!!! note
    If a value is a list, all elements in the list must be the same type.


## Examples

```py
await kv.set_key('config_path','path/file/config.json')
await kv.set_key('devices',['wlan0','sda','sdb'])
```

```py title='With Group'
# the group 'user_1234' would be a unique user ID
await kv.set_key('access_rights',['View','Add'], 'user_1234')
```
