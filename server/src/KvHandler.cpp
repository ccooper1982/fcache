#include <fc/KvHandler.hpp>
#include <fc/FlatBuffers.hpp>
#include <plog/Log.h>


namespace fc
{
  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVSet& req) noexcept
  { 
    bool valid = false;

    try
    {      
      const auto& keys = req.kv_flexbuffer_root().AsMap().Keys();
      const auto& values = req.kv_flexbuffer_root().AsMap().Values();
      const auto group = req.group();

      if (group && !group->empty())
      {
        if (const auto opt = getOrCreateGroup(group->str()); opt)
        {
          valid = setOrAdd<true>((*opt)->second.kv, keys, values);
        }
      }
      else
      {
        valid = setOrAdd<true>(keys, values);
      }        
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
    }

    createEmptyBodyResponse(fbb, valid ? Status_Ok : Status_Fail, ResponseBody_KVSet);
  }
  

  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVAdd& req) noexcept
  { 
    bool valid = false;
    try
    {
      const auto& keys = req.kv_flexbuffer_root().AsMap().Keys();
      const auto& values = req.kv_flexbuffer_root().AsMap().Values();

      if (const auto group = req.group(); group && !group->empty())
      {
        if (const auto opt = getOrCreateGroup(group->str()); opt)
        {
          valid = setOrAdd<false>((*opt)->second.kv, keys, values);
        }
      }
      else
      {
        valid = setOrAdd<false>(keys, values);
      }  
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
    }

    createEmptyBodyResponse(fbb, valid ? Status_Ok : Status_Fail, ResponseBody_KVAdd);
  }


  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVGet& req) noexcept
  {
    try
    {
      if (!req.keys())
      {
        createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_KVGet);
      }
      else
      {
        FlexBuilder flxb{4096U};

        if (const auto group = req.group(); group && !group->empty())
        {
          if (const auto opt = getGroup(group->str()); opt)
          {
            (*opt)->second.kv.get(*req.keys(), flxb);
          }
          else
          {
            // TODO Status_NotExist or
            flxb.Map([]{}); // best to return an existing but empty map 
          }          
        }
        else
        {
          m_map.get(*req.keys(), flxb);
        }

        flxb.Finish();

        const auto vec = fbb.CreateVector(flxb.GetBuffer());  // place the flex buffer vector in the flat buffer
        const auto body = fc::response::CreateKVGet(fbb, vec);
        
        auto rsp = fc::response::CreateResponse(fbb, Status_Ok, ResponseBody_KVGet, body.Union());
        fbb.Finish(rsp);
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << __FUNCTION__ << ":" << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_KVGet);
    }
  }


  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVRmv& req) noexcept
  {
    try
    {
      // API shouldn't permit non-existent vector but confirm
      if (req.keys())
      {
        const auto group = req.group();

        if (group && !group->empty())
        {
          if (const auto opt = getGroup(group->str()); opt)
          {
            (*opt)->second.kv.remove(*req.keys());
          }
        }
        else
        {
          m_map.remove(*req.keys());
        }
      }

      createEmptyBodyResponse(fbb, Status_Ok, ResponseBody_KVRmv);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_KVRmv);
    }
  }


  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVCount& req) noexcept
  {
    std::size_t count = 0;

    if (const auto group = req.group(); group && !group->empty())
    {
      if (const auto opt = getGroup(group->str()); opt)
      {
        count = (*opt)->second.kv.count();
      }
    }
    else
      count = m_map.count();
    
    const auto body = fc::response::CreateKVCount(fbb, count);
    const auto rsp = fc::response::CreateResponse(fbb, Status_Ok, ResponseBody_KVCount, body.Union());
    fbb.Finish(rsp);
  }

  
  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVContains& req) noexcept
  {
    if (!req.keys())
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_KVContains);
    else
    {
      FlexBuilder flxb;

      if (const auto group = req.group(); group && !group->empty())
      {
        if (const auto opt = getGroup(group->str()); opt)
        {
          (*opt)->second.kv.contains(flxb, *req.keys());
        }
        else
        {
          flxb.Map([]{});
        }
      }
      else
        m_map.contains(flxb, *req.keys());

      flxb.Finish();

      const auto vec = fbb.CreateVector(flxb.GetBuffer());
      const auto body = fc::response::CreateKVContains(fbb, vec);
        
      auto rsp = fc::response::CreateResponse(fbb, Status_Ok, ResponseBody_KVContains, body.Union());
      fbb.Finish(rsp);
    }
  }


  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVClear& req) noexcept
  {
    bool valid = true;

    try
    {
      switch (req.op())
      {
        using enum fc::request::ClearOperation;

        case ClearOperation_All:
          m_groups.clear();
          valid = m_map.clear();
        break;

        case ClearOperation_Groups:
          m_groups.clear();
        break;

        case ClearOperation_GroupsKeysOnly:
        {
          for (auto& group : m_groups)
            group.second.kv.clear();
        }
        break;

        case ClearOperation_GroupKeysOnly:
        case ClearOperation_Group:
        {
          if (const auto group = req.group(); group && !group->empty())
          {
            if (const auto opt = getGroup(group->str()); opt)
            {
              if (req.op() == ClearOperation_GroupKeysOnly)
                valid = (*opt)->second.kv.clear();
              else
                m_groups.erase(group->str());
            }
          }
        }
        break;
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      valid = false;
    }

    createEmptyBodyResponse(fbb, valid ? Status_Ok : Status_Fail, ResponseBody_KVClear);
  }
  
  
  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVClearSet& req) noexcept
  {
    bool valid = false;

    try
    {
      const auto& keys = req.kv_flexbuffer_root().AsMap().Keys();
      const auto& values = req.kv_flexbuffer_root().AsMap().Values();

      if (const auto group = req.group(); group && !group->empty())
      {
        if (const auto opt = getGroup(group->str()); opt)
        {
          (*opt)->second.kv.clear();
          valid = setOrAdd<true>((*opt)->second.kv, keys, values);
        }
      }
      else
      {
        m_map.clear();
        valid = setOrAdd<true>(keys, values);
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      valid = false;
    }

    createEmptyBodyResponse(fbb, valid ? Status_Ok : Status_Fail, ResponseBody_KVClearSet);
  }
}