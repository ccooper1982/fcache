# delete

```py
async def delete(names: typing.List[str]) -> None
```
Delete list(s).

To delete all lists without specifying each name, use [delete_all()](./delete_all.md).

## Examples

```py
await list.create('scores', type='int')
await list.create('usernames', type='str')
await list.delete(['scores', 'usernames'])
```