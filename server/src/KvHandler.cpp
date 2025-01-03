#include <fc/KvHandler.hpp>
#include <fc/FlatBuffers.hpp>
#include <plog/Log.h>


namespace fc
{

  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVSet& set) noexcept
  { 
    try
    {
      const auto& vec = set.kv_flexbuffer_root().AsMap().Values();
      const auto& keys = set.kv_flexbuffer_root().AsMap().Keys();

      for (std::size_t i = 0 ; i < vec.size(); ++i)
      {
        const auto& key = keys[i].AsString().str();

        switch (vec[i].GetType())
        {
          using enum flexbuffers::Type;

          case FBT_INT:
            m_map.set<FBT_INT>(key, vec[i].AsInt64());
          break;
          
          case FBT_UINT:
            m_map.set<FBT_UINT>(key, vec[i].AsUInt64());
          break;

          case FBT_BOOL:
            m_map.set<FBT_BOOL>(key, vec[i].AsBool());
          break;

          case FBT_STRING:
            m_map.set<FBT_STRING>(key, vec[i].AsString().str());
          break;

          case FBT_FLOAT:
            m_map.set<FBT_FLOAT>(key, vec[i].AsFloat());
          break;

          default:
            PLOGE << __FUNCTION__ << " - unknown type";
          break;
        }
      }

      createEmptyBodyResponse(fbb, Status_Ok, ResponseBody_KVSet);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_KVSet);
    }
  }
  

  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVGet& get) noexcept
  {
    try
    {
      if (!get.keys())
      {
        createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_KVGet);
      }
      else
      {
        FlexBuilder flxb;
        m_map.get(*get.keys(), flxb);
        flxb.Finish();

        const auto buff = flxb.GetBuffer();

        const auto vec = fbb.CreateVector(buff);  // place the flex buffer vector in the flat buffer
        const auto body = fc::response::CreateKVGet(fbb, vec);
        
        auto rsp = fc::response::CreateResponse(fbb, Status_Ok, ResponseBody_KVGet, body.Union());
        fbb.Finish(rsp);
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_KVGet);
    }
  }


  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVRmv& rmv) noexcept
  {
    try
    {
      if (rmv.keys())
        m_map.remove(*rmv.keys());

      createEmptyBodyResponse(fbb, Status_Ok, ResponseBody_KVRmv);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_KVRmv);
    }
  }


  void KvHandler::createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType) noexcept
  {
    try
    {
      const auto rsp = fc::response::CreateResponse (fbb, status, bodyType);
      fbb.Finish(rsp);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
    }
  }
}