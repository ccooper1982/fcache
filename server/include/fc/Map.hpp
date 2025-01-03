#pragma once

#include <ankerl/unordered_dense.h>
#include <fc/KvCommon.hpp>

namespace fc
{
  class CacheMap
  {
    using Map = ankerl::unordered_dense::segmented_map<CachedKey, CachedValue>;
    using CacheMapIterator = Map::iterator;
    using CacheMapConstIterator = Map::const_iterator;

  public:
    
    CacheMap& operator=(CacheMap&&) = default; // required by Map::erase()
    CacheMap(CacheMap&&) = default;

    CacheMap& operator=(const CacheMap&) = delete;   
    CacheMap(CacheMap&) = delete;

    
    CacheMap (const std::size_t buckets = 0) : m_map(buckets) 
    {
    }


    template<bool IsSet, flexbuffers::Type FlexT, typename ValueT>
    void setOrAdd (const CachedKey& key, const ValueT& v)
    {
      ValueExtractF extract{nullptr};

      if constexpr (FlexT == flexbuffers::Type::FBT_INT)
        extract = extractInt;
      else if constexpr (FlexT == flexbuffers::Type::FBT_UINT)
        extract = extractUInt;
      else if constexpr (FlexT == flexbuffers::Type::FBT_FLOAT)
        extract = extractFloat;
      else if constexpr (FlexT == flexbuffers::Type::FBT_BOOL)
        extract = extractBool;
      else if constexpr (FlexT == flexbuffers::Type::FBT_STRING)
        extract = extractString;
      else
        static_assert("Unsupported FlexBuffer::Type");

      if constexpr (IsSet)
        m_map.insert_or_assign(key, CachedValue{v, extract});
      else
        m_map.try_emplace(key, v, extract);
    }


    void get (const KeyVector& keys, FlexBuilder& fb)
    {
      fb.Map([&]()
      {
        for (const auto& key : keys)
        { 
          if (const auto& it = m_map.find(key->str()); it != m_map.cend())
          {
            const auto pKey = key->c_str();
            const auto& cachedValue = it->second;
            
            cachedValue.extract(pKey, cachedValue.value, fb);
          }
        }      
      });
    }


    void remove (const KeyVector& keys)
    {
      for (const auto& key : keys)
      {
        m_map.erase(key->str());
      }
    };

    
    std::tuple<bool, std::size_t> clear()
    {
      // auto size = m_map.size();
      // bool valid = true;

      // try
      // {
      //   m_map.replace(Map::value_container_type{});
      // }
      // catch (...)
      // {
      //   valid = false;
      //   size = 0U;
      // }
      
      // return std::make_pair(valid, size);

      return {false, 0};
    };


    std::size_t count() const
    {
      return m_map.size();
    }

    
    flatbuffers::Offset<KeyVector> contains (FlatBuilder& fb, const KeyVector& keys) const
    {
      // Building a flatbuffer vector, we need to know the length of the vector at construction.
      // We don't know that until we've checked which keys exist.
      // There may be a better way of doing this.
      std::vector<CachedKey> keysExist;
      keysExist.reserve(keys.size());

      for (const auto& key : keys)
      {
        const auto& sKey = key->str();

        if (m_map.contains(sKey))
          keysExist.emplace_back(sKey);
      }
      
      return fb.CreateVectorOfStrings(keysExist.cbegin(), keysExist.cend());
    };


    const Map& map () const
    {
      return m_map;
    }


  private:

    static void extractInt(const char * key, const ValueVariant& value, FlexBuilder& fb)
    {
      fb.Int(key, std::get<CachedValue::GET_INT>(value));
    }

    static void extractUInt(const char * key, const ValueVariant& value, FlexBuilder& fb)
    {
      fb.UInt(key, std::get<CachedValue::GET_UINT>(value));
    }

    static void extractFloat(const char * key, const ValueVariant& value, FlexBuilder& fb)
    {
      fb.Float(key, std::get<CachedValue::GET_DBL>(value));
    }

    static void extractBool(const char * key, const ValueVariant& value, FlexBuilder& fb)
    {
      fb.Bool(key, std::get<CachedValue::GET_BOOL>(value));
    }

    static void extractString(const char * key, const ValueVariant& value, FlexBuilder& fb)
    {
      fb.String(key, std::get<CachedValue::GET_STR>(value));
    }


  private:
    Map m_map;
  };

}