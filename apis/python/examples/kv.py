import asyncio as asio
import random
import array
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

  ## str
  await kv.set({'s':'hello'})
  print(await kv.get(keys=['s']))

  #vector
  
  # await kv.set({'i':createIntArray([123,456,34])})
  # print(await kv.get(key='i'))

  # await kv.set({'f':createFloatArray([12.34, 56.78])})
  # print(await kv.get(key='f'))

  # await kv.set({'list':['asda','adsa']})
  # print(await kv.get(keys=['list']))

  # fixed to vector
  # await kv.set({'x':20})
  # print(await kv.get(key='x'))
  # await kv.set({'x':createIntArray([123,456,34])})
  # print(await kv.get(key='x'))
  # await kv.set({'x':30})
  # print(await kv.get(key='x'))

  
  # ## add
  await kv.add({'a1':456}) # new key
  print(await kv.get(keys=['a1']))

  await kv.add({'a2':456}) # attempt overwrite existing
  print(await kv.get(keys=['a2']))


  # # remove
  # await kv.set({'k1':123, 'k2':234})
  # print(await kv.get(keys=['k1','k2']))

  # await kv.remove(key='k2')
  # print(await kv.get(keys=['k1','k2']))

  # await kv.set({'k2':234})
  # print(await kv.get(keys=['k1','k2']))


  # clear


async def more():
  if (client := await connect()) is None:
    return
  
  kv = KV(client)

  data = {}
  for i in range(500):
    data[f'k{i}'] = i

  await kv.set(data)
  print(await kv.count())

  keys = []
  for _ in range(20):
    keys.append(f'k{random.randrange(0, 20)}')
  print(await kv.get(keys=keys))


async def lists():
  if (client := await connect()) is None:
    return
  
  kv = KV(client)

  # strings
  await kv.set({'s':['hello', 'wurluld']})
  print(await kv.get(key='s'))


  # int
  await kv.set({'i':[123,456]})
  print(await kv.get(key='i'))

  # float
  await kv.set({'f':[12.5,45.5]})
  print(await kv.get(key='f'))

  # bool
  await kv.set({'b':[True, False, True]})
  print(await kv.get(key='b'))

  # empty:
  try:
    await kv.set({'s_empty':[]})
  except ValueError as ve:
    print(f'Good fail: {ve}')


async def blob():
  # Requires fcache is started with maxPayload to size of the cat image which is 11,030 bytes
  # ./fcache --maxPayload=16384
  if (client := await connect()) is None:
    return
  
  file_data = bytes()
  with open('cat.jpg', mode='rb') as file:
    file_data = file.read()

  kv = KV(client)

  await kv.set({'img_cat':file_data})
  
  if data := await kv.get(key='img_cat'):
    with open('cat_rsp.jpg', mode='wb') as file:
      file.write(data)



if __name__ == "__main__":
  async def run():
    for f in [blob]:
      print(f'---- {f.__name__} ----')
      await f()
  
  asio.run(run())