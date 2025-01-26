#include <fc/KvHandler.hpp>
#include <fc/FlatBuffers.hpp>
#include <plog/Log.h>


namespace fc
{
  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVSet& set) noexcept
  { 
    bool valid = false;

    try
    {      
      const auto& keys = set.kv_flexbuffer_root().AsMap().Keys();
      const auto& values = set.kv_flexbuffer_root().AsMap().Values();

      valid = setOrAdd<true>(keys, values);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
    }

    createEmptyBodyResponse(fbb, valid ? Status_Ok : Status_Fail, ResponseBody_KVSet);
  }
  

  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVAdd& add) noexcept
  { 
    bool valid = false;
    try
    {
      const auto& keys = add.kv_flexbuffer_root().AsMap().Keys();
      const auto& values = add.kv_flexbuffer_root().AsMap().Values();

      valid = setOrAdd<false>(keys, values);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
    }

    createEmptyBodyResponse(fbb, valid ? Status_Ok : Status_Fail, ResponseBody_KVAdd);
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
      PLOGE << __FUNCTION__ << ":" << e.what();
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


  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVCount& count) noexcept
  {
    const auto body = fc::response::CreateKVCount(fbb, m_map.count());
    const auto rsp = fc::response::CreateResponse(fbb, Status_Ok, ResponseBody_KVCount, body.Union());
    fbb.Finish(rsp);
  }

  
  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVContains& contains) noexcept
  {
    if (!contains.keys())
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_KVContains);
    else
    {
      FlexBuilder flxb;
      m_map.contains(flxb, *contains.keys());
      flxb.Finish();

      const auto vec = fbb.CreateVector(flxb.GetBuffer());
      const auto body = fc::response::CreateKVContains(fbb, vec);
        
      auto rsp = fc::response::CreateResponse(fbb, Status_Ok, ResponseBody_KVContains, body.Union());
      fbb.Finish(rsp);
    }
  }


  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVClear& clear) noexcept
  {
    bool valid = false;
    try
    {
      valid = m_map.clear();
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
    }

    createEmptyBodyResponse(fbb, valid ? Status_Ok : Status_Fail, ResponseBody_KVClear);
  }
  
  
  void KvHandler::handle(FlatBuilder& fbb, const fc::request::KVClearSet& clearSet) noexcept
  {
    try
    {
      m_map.clear();

      const auto& keys = clearSet.kv_flexbuffer_root().AsMap().Keys();
      const auto& values = clearSet.kv_flexbuffer_root().AsMap().Values();

      const auto valid = setOrAdd<false>(keys, values);

      createEmptyBodyResponse(fbb, valid ? Status_Ok : Status_Fail, ResponseBody_KVClearSet);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_KVClearSet);
    }
  }


  void KvHandler::createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType) noexcept
  {
    // TODO this function is also in ListHandler. Move to Common.hpp or create Handler base class
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