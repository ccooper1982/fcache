#pragma once

#include <fc/FlatBuffers.hpp>
#include <fc/Map.hpp>
#include <plog/Log.h>

namespace fc
{
  class KvHandler
  {
    struct Group
    {
      CacheMap kv;
    };

    using GroupMap = ankerl::unordered_dense::map<std::string, Group>;
  
  public:
    KvHandler() = default;


  public:
    void handle(FlatBuilder& fbb, const fc::request::KVSet& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVAdd& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVGet& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVRmv& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVCount& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVContains& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVClear& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVClearSet& req) noexcept;


  private:
    template<bool IsSet>
    bool setOrAdd (CacheMap& map, const flexbuffers::TypedVector& keys, const flexbuffers::Vector& values)
    {
      bool valid = true;
      for (std::size_t i = 0 ; i < values.size() && valid; ++i)
      {
        const auto& key = keys[i].AsString().str();
        
        switch (values[i].GetType())
        {
          using enum FlexType;

          case FBT_INT:
            valid = map.setOrAdd<IsSet, FBT_INT>(key, values[i].AsInt64());
          break;
          
          case FBT_UINT:
            valid = map.setOrAdd<IsSet, FBT_UINT>(key, values[i].AsUInt64());
          break;

          case FBT_BOOL:
            valid = map.setOrAdd<IsSet, FBT_BOOL>(key, values[i].AsBool());
          break;

          case FBT_FLOAT:
            valid = map.setOrAdd<IsSet, FBT_FLOAT>(key, values[i].AsFloat());
          break;

          case FBT_STRING:
            valid = map.setOrAdd<IsSet>(key, values[i].AsString().c_str());
          break;

          case FBT_BLOB:
            valid = map.setOrAdd<IsSet>(key, values[i].AsBlob());
          break;

          case FBT_VECTOR_INT:
            valid = map.setOrAdd<IsSet, FBT_VECTOR_INT>(key, values[i].AsTypedVector());
          break;

          case FBT_VECTOR_UINT:
            valid = map.setOrAdd<IsSet, FBT_VECTOR_UINT>(key, values[i].AsTypedVector());
          break;

          case FBT_VECTOR_FLOAT:
            valid = map.setOrAdd<IsSet, FBT_VECTOR_FLOAT>(key, values[i].AsTypedVector());
          break;

          case FBT_VECTOR_BOOL:
            valid = map.setOrAdd<IsSet, FBT_VECTOR_BOOL>(key, values[i].AsTypedVector());
          break;

          case FBT_VECTOR_KEY:  // for vector of strings
            valid = map.setOrAdd<IsSet, FBT_VECTOR_KEY>(key, values[i].AsTypedVector());
          break;

          default:
            PLOGE << __FUNCTION__ << " - unsupported type: " << values[i].GetType();
            valid = false;
          break;
        }
      }

      return valid;
    }


    template<bool IsSet>
    bool setOrAdd (const flexbuffers::TypedVector& keys, const flexbuffers::Vector& values)
    {
      return setOrAdd<IsSet>(m_map, keys, values);
    }


    std::optional<GroupMap::iterator> getGroup (const std::string& name)
    {
      if (const auto it = m_groups.find(name) ; it == m_groups.end())
        return {};
      else
        return it;
    }


    std::optional<GroupMap::iterator> getOrCreateGroup (const std::string& name)
    {
      if (const auto it = m_groups.find(name) ; it == m_groups.end())
        return createGroup(name);
      else
        return it;
    }


    GroupMap::iterator createGroup (const std::string& name)
    {
      const auto it = m_groups.try_emplace(name, CacheMap{});
      return it.first;
    }


  private:
    CacheMap m_map;
    GroupMap m_groups;
  };
}