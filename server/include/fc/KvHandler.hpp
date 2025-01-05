#pragma once

#include <fc/FlatBuffers.hpp>
#include <fc/Map.hpp>
#include <plog/Log.h>

namespace fc
{
  class KvHandler
  {
    using enum fc::response::Status;
    using enum fc::response::ResponseBody;


  public:
    KvHandler() = default;


  public:
    void handle(FlatBuilder& fbb, const fc::request::KVSet& set) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVAdd& add) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVGet& get) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVRmv& rmv) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVCount& count) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVContains& contains) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVClear& clear) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVClearSet& clearSet) noexcept;


  private:
    void createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType) noexcept;


    template<bool IsSet>
    bool setOrAdd (const flexbuffers::TypedVector& keys, const flexbuffers::Vector& values)
    {
      bool valid = true;
      for (std::size_t i = 0 ; i < values.size(); ++i)
      {
        const auto& key = keys[i].AsString().str();

        switch (values[i].GetType())
        {
          using enum flexbuffers::Type;

          case FBT_INT:
            m_map.setOrAdd<IsSet, FBT_INT>(key, values[i].AsInt64());
          break;
          
          case FBT_UINT:
            m_map.setOrAdd<IsSet, FBT_UINT>(key, values[i].AsUInt64());
          break;

          case FBT_BOOL:
            m_map.setOrAdd<IsSet, FBT_BOOL>(key, values[i].AsBool());
          break;

          case FBT_STRING:
            m_map.setOrAdd<IsSet, FBT_STRING>(key, values[i].AsString().str());
          break;

          case FBT_FLOAT:
            m_map.setOrAdd<IsSet, FBT_FLOAT>(key, values[i].AsFloat());
          break;

          default:
            PLOGE << __FUNCTION__ << " - unsupported type";
            valid = false;
          break;
        }
      }
      return valid;
    }


  private:
    CacheMap m_map;
  };

}