import asyncio as asio
import random
import sys
sys.path.append('../')
import fc
from fc.kv import KV
from fc.list import UnsortedList


async def connect() -> fc.Client:
  try:
    return await fc.fcache('ws://127.0.0.1:1987')
  except:
    print ('Failed to connect')
  return None



class Users:
  def __init__(self, client: fc.Client):
    self.kv_api = KV(client)
    
  async def reset(self):
    await self.kv_api.clear()
    
  
  async def create(self, email: str, handle:str, fname:str, sname:str, profile_pic:bytes):
    data = {'handle':handle, 'fname':fname,'sname':sname, 'profile_img':profile_pic, 'status':'AuthRequired'}
    await self.kv_api.set(data, group=email)
    
  async def set_authed(self, email: str):
    await self.kv_api.set({'status':'Authed'}, group=email)
    
  async def download_profile_img(self, email: str, dir:str):
    print(f'Downloading profile image for {email} ... ')

    if data := await self.kv_api.get(key='profile_img', group=email):
      path = f'{dir}/{email}_img.jpg'
      with open(path, mode='wb') as file:
        file.write(data)
        print(f'Saved {path}')
    
  async def get_profile(self, email:str) -> dict:
    return await self.kv_api.get_all(email)


class Tasks:
  def __init__(self, client: fc.Client):
    self.list_api = UnsortedList(client)

  async def reset(self):
    await self.list_api.delete_all()

  async def create_tasks_list(self, email:str):
    await self.list_api.create(email, type='str')

  async def add_task(self, email:str, task:str, to_head: bool = False):
    if to_head:
      await self.list_api.add_head(email,[task])
    else:
      await self.list_api.add_tail(email,[task])

  async def get_tasks(self, email:str):
    return await self.list_api.get_n(email)



async def dump_user(users:Users, tasks: Tasks, email: str):
  profile = await users.get_profile(email)
  taskList = await tasks.get_tasks(email)

  print(f'----- Profile -----')
  print(f'Email: {email}')

  for k,v in profile.items():
    if k != 'profile_img':
      print(f'{k} : {v}')
  
  print(f'----- Tasks -----')
  
  for task in taskList:
    print(task)

  print(f'==============')



async def go():
  if (client := await connect()) is None:
    return
    

  cat_data = bytes()
  with open('cat.jpg', mode='rb') as file:
    cat_data = file.read()

  dog_data = bytes()
  with open('dog.jpg', mode='rb') as file:
    dog_data = file.read()

  bob_email = 'bob@email.com'
  fred_email = 'fred@email.com'


  users = Users(client)
  tasks = Tasks(client)

  await users.reset()
  await tasks.reset()

  await users.create(bob_email, 'User1', 'Bob', 'Cat', cat_data)
  await tasks.create_tasks_list(bob_email)
  await tasks.add_task(bob_email, 'Buy cat food')

  await users.create(fred_email, 'User2', 'Fred', 'Dog', dog_data)
  await tasks.create_tasks_list(fred_email)
  await tasks.add_task(fred_email, 'Walkies')
  await tasks.add_task(fred_email, 'Buy waste bags', to_head=True)
  
  await users.set_authed(bob_email)

  await dump_user(users, tasks, bob_email)
  await dump_user(users, tasks, fred_email)

  await users.download_profile_img(fred_email, './')


if __name__ == "__main__":
  asio.run(go())