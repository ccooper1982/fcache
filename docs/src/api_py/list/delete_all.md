# delete_all

```py
async def delete_all() -> None
```
Delete all lists.

To delete all single or multiple lists by name, use [delete()](./delete.md).

## Examples

```py
await list.create('scores', type='int')
await list.create('usernames', type='str')
await list.delete_all()
```