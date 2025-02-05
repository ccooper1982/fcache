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

    template<bool IsSet>
    bool setOrAdd (const flexbuffers::TypedVector& keys, const flexbuffers::Vector& values)
    {
      bool valid = true;
      for (std::size_t i = 0 ; i < values.size() && valid; ++i)
      {
        const auto& key = keys[i].AsString().str();
        
        switch (values[i].GetType())
        {
          using enum FlexType;

          case FBT_INT:
            valid = m_map.setOrAdd<IsSet, FBT_INT>(key, values[i].AsInt64());
          break;
          
          case FBT_UINT:
            valid = m_map.setOrAdd<IsSet, FBT_UINT>(key, values[i].AsUInt64());
          break;

          case FBT_BOOL:
            valid = m_map.setOrAdd<IsSet, FBT_BOOL>(key, values[i].AsBool());
          break;

          case FBT_FLOAT:
            valid = m_map.setOrAdd<IsSet, FBT_FLOAT>(key, values[i].AsFloat());
          break;

          case FBT_STRING:
            valid = m_map.setOrAdd<IsSet>(key, values[i].AsString().c_str());
          break;

          case FBT_BLOB:
            valid = m_map.setOrAdd<IsSet>(key, values[i].AsBlob());
          break;

          case FBT_VECTOR_INT:
            valid = m_map.setOrAdd<IsSet, FBT_VECTOR_INT>(key, values[i].AsTypedVector());
          break;

          case FBT_VECTOR_UINT:
            valid = m_map.setOrAdd<IsSet, FBT_VECTOR_UINT>(key, values[i].AsTypedVector());
          break;

          case FBT_VECTOR_FLOAT:
            valid = m_map.setOrAdd<IsSet, FBT_VECTOR_FLOAT>(key, values[i].AsTypedVector());
          break;

          case FBT_VECTOR_BOOL:
            valid = m_map.setOrAdd<IsSet, FBT_VECTOR_BOOL>(key, values[i].AsTypedVector());
          break;

          case FBT_VECTOR_KEY:  // for vector of strings
            valid = m_map.setOrAdd<IsSet, FBT_VECTOR_KEY>(key, values[i].AsTypedVector());
          break;

          default:
            PLOGE << __FUNCTION__ << " - unsupported type: " << values[i].GetType();
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