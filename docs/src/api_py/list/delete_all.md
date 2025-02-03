# delete_all

```py
async def delete_all() -> None
```
Delete all lists.

To delete particular list(s) by name, use [delete()](./delete.md).

## Examples

```py
await list.create('scores', type='int')
await list.create('usernames', type='str')
await list.create('foo', type='int')
await list.delete_all()
```