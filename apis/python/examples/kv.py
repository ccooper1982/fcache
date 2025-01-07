import asyncio as asio
import sys
sys.path.append('../')
import fc
from fc.kv import KV

async def connect() -> fc.Client:
  try:
    return await fc.fcache('ws://127.0.0.1:1987')
  except:
    print ('Failed to connect')
  
  return None


async def test():
  if (client := await connect()) is None:
    return

  try:
    data = {'k1':123, 'k2':'hello', 'k3':123.5, 'k4':-123, 'k5':False,'k6':True}
    kv = KV(client)

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
        

async def test2():
  if (client := await connect()) is None:
    return
  
  kv = KV(client)
  
  ## set

  # flip flop keys' value types
  await kv.set({'k1':123, 'k2':234})
  print(await kv.get(keys=['k1','k2']))

  await kv.set({'k1':True, 'k2':False})
  print(await kv.get(keys=['k1','k2']))
  
  await kv.set({'k1':123, 'k2':234})
  print(await kv.get(keys=['k1','k2']))

  await kv.set({'k1':True, 'k2':False})
  print(await kv.get(keys=['k1','k2']))

  #decimal
  await kv.set({'f':123.5})
  print(await kv.get(keys=['f']))


  ## add
  await kv.add({'a1':456}) # new key
  print(await kv.get(keys=['a1']))

  await kv.add({'k1':456}) # attempt overwrite existing
  print(await kv.get(keys=['k1']))


if __name__ == "__main__":
  async def run():
    for f in [test2]:
      print(f'---- {f.__name__} ----')
      await f()
  
  asio.run(run())