import asyncio as asio
import sys
sys.path.append('../')
from fc.client import NdbClient
from fc.kv import KV

async def test():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987')
  
  data = {'k1':123, 'k2':'hello', 'k3':123.456, 'k4':-123, 'k5':False,'k6':True}
    
  kv = KV(client)

  try:
    await kv.set(data)
    print(await kv.get(keys=['k1','k2','k3','k4','k5', 'k6']))

    # overwrite
    await kv.set({'k1':321})
    print(await kv.get(keys=['k1','k2','k3','k4','k5', 'k6']))

    await kv.remove(keys=['k1'])
    print(await kv.get(keys=['k1','k2','k3','k4','k5', 'k6']))

    # does not overwrite, so should just add k1 since we removed it
    data['k2'] = 'world' # 'k2' should not change
    await kv.add(data)
    print(await kv.get(keys=['k1','k2','k3','k4','k5', 'k6']))
    
    print(await kv.count())

    print(await kv.contains(['k1', 'k2', '__' ]))

    await kv.clear_set({'k10':123})
    print(await kv.get(keys=['k10']))
    
    await kv.clear()
    print(await kv.get(keys=['k10']))

  except:
    print('Query failed')
        


if __name__ == "__main__":
  for f in [test()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)