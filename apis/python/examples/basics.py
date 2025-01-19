import asyncio as asio
import sys
sys.path.append('../')
import fc
from fc.kv import KV


async def connect() -> fc.Client:
  try:
    client = await fc.fcache(uri='ws://127.0.0.1:1987')
    # or: client = await fc.fcache(ip='127.0.0.1', port=1987)
  except:
    print ('Failed to connect')
    client = None
  return client


async def kv():
  if (client := await connect()) is None:
    return
  
  # create API object for KV commands
  kv = KV(client)

  await kv.set({'player':'Monster',
                'level':25,
                'active':True,
                'perks':['Armour','Kilt']})

  # get single key, returns the value (or None if key not found)
  age = await kv.get(key='level')
  print(f'Age: {age}')

  # get multiple keys, returns dict
  rsp = await kv.get(keys=['player', 'active'])
  print(f"Player: {rsp['player']}, Active: {rsp['active']}")

  # get list
  print(await kv.get(key='perks'))


async def kv_blob():
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
      print('Image written')


if __name__ == "__main__":
  async def run():
    for f in [kv, kv_blob]:
      print(f'---- {f.__name__} ----')
      await f()
  
  asio.run(run())