#pragma once

#include <ankerl/unordered_dense.h>
#include <fc/KvCommon.hpp>
#include <plog/Log.h>


namespace fc
{
  class CacheMap
  {
    using Map = ankerl::unordered_dense::pmr::map<std::string, CachedValue>;
    using CacheMapIterator = Map::iterator;
    using CacheMapConstIterator = Map::const_iterator;
    using enum FlexType;


  public:
    CacheMap() : m_map(std::pmr::polymorphic_allocator {fc::MapMemory::getPool()})
    {

    }

    CacheMap& operator=(CacheMap&&) = default;
    CacheMap(CacheMap&&) = default;

    CacheMap& operator=(const CacheMap&) = delete;
    CacheMap(CacheMap&) = delete;


    template<bool IsSet, FlexType FlexT, typename ValueT>
    bool setOrAdd (const CachedKey& key, const ValueT& value) noexcept requires (std::is_integral_v<ValueT> || std::is_same_v<ValueT, float>)
    {
      try
      {
        ExtractFixedF extract{nullptr};

        if constexpr (FlexT == FlexType::FBT_INT)
          extract = extractInt;
        else if constexpr (FlexT == FlexType::FBT_UINT)
          extract = extractUInt;
        else if constexpr (FlexT == FlexType::FBT_FLOAT)
          extract = extractFloat;
        else if constexpr (FlexT == FlexType::FBT_BOOL)
          extract = extractBool;
        else
          static_assert(false, "FlexBuffers::Type not supported for fixed type");

        // set and add commands both emplace if does not exist
        if (const auto it = m_map.find(key) ; it == m_map.end())
        {
          m_map.try_emplace(key, FixedValue{value, extract});
        }
        else if constexpr (IsSet)
        {
          // but only set overwrites existing
          it->second.value = FixedValue {value, extract};
          it->second.valueType = CachedValue::FIXED;
        }
        return true;
      }
      catch(const std::exception& ex)
      {
        PLOGE << ex.what();
      }
      
      return false;
    }


    template<bool IsSet, FlexType FlexT>
    bool setOrAdd (const CachedKey& key, const flexbuffers::TypedVector& v) noexcept
    {
      try
      {
        if (const auto it = m_map.find(key) ; it == m_map.end())
        {
          storeVectorValue<FlexT>(key, v);
        }
        else if (IsSet)
        {
          // key already exists, so only replace value if it's a set command
          it->second.valueType = CachedValue::VEC;
          it->second.value = VectorValue{};
          storeVectorValue<FlexT>(it, v);
        }
      }
      catch(const std::exception& e)
      {
        PLOGE << __FUNCTION__ << ":" << e.what();
        return false;
      }

      return true;
    }


    template<bool IsSet>
    bool setOrAdd (const CachedKey& key, const std::string_view str) noexcept
    {
      try
      {
        if (const auto it = m_map.find(key) ; it == m_map.end())
        {
          auto [itEmplaced, _] = m_map.try_emplace(key, VectorValue{});
          stringToMap(itEmplaced, str);
        }
        else if (IsSet)
        {
          it->second.valueType = CachedValue::VEC;
          it->second.value = VectorValue{};

          stringToMap(it, str);
        }
      }
      catch(const std::exception& e)
      {
        PLOGE << __FUNCTION__ << ":" << e.what();
        return false;
      }

      return true;
    }


    template<bool IsSet>
    bool setOrAdd (const CachedKey& key, const flexbuffers::Blob& blob)
    {
      try
      {
        if (const auto it = m_map.find(key) ; it == m_map.end())
        {
          auto [itEmplaced, _] = m_map.try_emplace(key, VectorValue{});
          blobToMap(itEmplaced, blob);
        }
        else if (IsSet)
        {
          it->second.valueType = CachedValue::VEC;
          it->second.value = VectorValue{};
          blobToMap(it, blob);
        }
      }
      catch(const std::exception& e)
      {
        PLOGE << __FUNCTION__ << ":" << e.what();
        return false;
      }

      return true;
    }


    inline void get (const KeyVector& keys, FlexBuilder& fb)
    {
      fb.Map([&]()
      {
        for (const auto& key : keys)
        { 
          if (const auto& it = m_map.find(key->str()); it != m_map.cend())
          {
            const auto pKey = key->c_str();
            const auto& cachedValue = it->second;

            if (cachedValue.valueType == CachedValue::FIXED)
            {
              const auto& fixedValue = std::get<FixedValue>(cachedValue.value);
              fixedValue.extract(fb, pKey, fixedValue);
            }
            else if (cachedValue.valueType == CachedValue::VEC)
            {
              const auto& vecValue = std::get<VectorValue>(cachedValue.value);
              vecValue.extract(fb, pKey, vecValue);
            }
          }
        }      
      }); 
    }

    
    // Get all keys in map
    inline void get(FlexBuilder& fb)
    {
      fb.Map([&]
      {
        for (const auto& kv : m_map)
        {
          const auto& cachedValue = kv.second;
          const auto& key = kv.first;

          if (cachedValue.valueType == CachedValue::FIXED)
          {
            const auto& fixedValue = std::get<FixedValue>(cachedValue.value);
            fixedValue.extract(fb, key.c_str(), fixedValue);
          }
          else if (cachedValue.valueType == CachedValue::VEC)
          {
            const auto& vecValue = std::get<VectorValue>(cachedValue.value);
            vecValue.extract(fb, key.c_str(), vecValue);
          }
        }
      });
    }


    void remove (const KeyVector& keys)
    {
      for (const auto& key : keys)
        m_map.erase(key->str());
    };

    
    bool clear() noexcept
    {
      try
      {
        m_map.clear();//replace(Map::value_container_type{});
        PLOGD << __FUNCTION__ << ":" << m_map.size();
        return true;
      }
      catch (const std::exception& ex)
      {
        PLOGE << ex.what();
        return false;
      }
    };


    void contains (FlexBuilder& fb, const KeyVector& keys) const
    {
      fb.TypedVector([&keys, &fb, this]()
      {
        for (const auto key : keys)
        {
          if (m_map.contains(key->str()))
            fb.String(key->c_str());
        }
      });      
    }


    std::size_t count() const noexcept
    {
      return m_map.size();
    }


  private:

    static void extractInt(FlexBuilder& fb, const char * key, const FixedValue& fv);
    static void extractUInt(FlexBuilder& fb, const char * key, const FixedValue& fv);
    static void extractFloat(FlexBuilder& fb, const char * key, const FixedValue& fv);
    static void extractBool(FlexBuilder& fb, const char * key, const FixedValue& fv);
    static void extractBlob(FlexBuilder& fb, const char * key, const VectorValue& fv);
    static void extractStringV(FlexBuilder& fb, const char * key, const VectorValue& vv);
    static void extractIntV(FlexBuilder& fb, const char * key, const VectorValue& vv);
    static void extractUIntV(FlexBuilder& fb, const char * key, const VectorValue& vv);
    static void extractFloatV(FlexBuilder& fb, const char * key, const VectorValue& vv);
    static void extractBoolV(FlexBuilder& fb, const char * key, const VectorValue& vv);
    static void extractString(FlexBuilder& fb, const char * key, const VectorValue& vv);
    

    template<FlexType FlexT>
    void storeVectorValue (const std::string& key, const flexbuffers::TypedVector& v)
    {
      // this function is only called if the key is not in the map, so
      // no need to check second return value of try_emplace()
      auto [it, _] = m_map.try_emplace(key, VectorValue{});
      storeVectorValue<FlexT>(it, v);
    }


    template<FlexType FlexT>
    void storeVectorValue (const Map::iterator it, const flexbuffers::TypedVector& v)
    {
      if constexpr (FlexT == FBT_VECTOR_INT)
        scalarsToMap<int64_t>(it, v, FlexT, extractIntV) ;
      else if constexpr (FlexT == FBT_VECTOR_UINT)
        scalarsToMap<uint64_t>(it, v, FlexT, extractUIntV) ;
      else if constexpr (FlexT == FBT_VECTOR_BOOL)
        scalarsToMap<bool>(it, v, FlexT, extractBoolV) ;
      else if constexpr (FlexT == FBT_VECTOR_FLOAT)
        scalarsToMap<float>(it, v, FlexT, extractFloatV) ;
      else if constexpr (FlexT == FBT_VECTOR_KEY) // vector of strings
        stringsToMap(it, v);
    }


    template<typename ScalarT>
    void scalarsToMap (const Map::iterator it, const flexbuffers::TypedVector& v,
                      const FlexType flexType, const ExtractVectorF extract)
    {
      const std::size_t size = sizeof(ScalarT) * v.size();
      
      auto& vec = std::get<CachedValue::VEC>(it->second.value);      
      vec.type = flexType;
      vec.extract = extract;
      vec.data.resize(size);

      for (std::size_t i = 0, j = 0 ; i < v.size() ; ++i)
      {
        const auto val = v[i].As<ScalarT>();
        std::memcpy(vec.data.data()+j, &val, sizeof(ScalarT));
        j += sizeof(ScalarT);
      }
    }


    void stringsToMap(const Map::iterator it, const flexbuffers::TypedVector& v)
    {
      // strings are appended together, using the null terminator as a delimiter

      // get total length of all strings
      std::size_t totalLength{0};
      for (std::size_t s = 0 ; s < v.size() ; ++s)
        totalLength += v[s].AsString().length();

      auto& vec = std::get<CachedValue::VEC>(it->second.value);
      vec.type = FBT_VECTOR_KEY;
      vec.extract = extractStringV;
      vec.data.resize(totalLength + v.size());

      auto buffer = vec.data.data();

      // for each string
      std::size_t dest = 0;

      for (std::size_t s = 0 ; s < v.size() ; ++s)
      {
        const auto len = v[s].AsString().length();
        
        std::memcpy(buffer+dest, v[s].AsString().c_str(), len);
        dest += len;
        buffer[dest++] = '\0';        
      }
    }


    void stringToMap(const Map::iterator it, const std::string_view& str)
    {
      const std::size_t totalLength = str.size() + 1; // +1 for '\0'
      
      auto& vec = std::get<CachedValue::VEC>(it->second.value);
      vec.type = FBT_STRING;
      vec.extract = extractString;
      vec.data.resize(totalLength);

      auto buffer = vec.data.data();
      std::memcpy(buffer, str.data(), str.size());
      buffer[str.size()] = '\0';
    }


    void blobToMap(const Map::iterator it, const flexbuffers::Blob& blob)
    {
      // [blob_len][blob_data]
      // where blob_len is fcblobsize (uint32_t)

      const fcblobsize blobSize = static_cast<fcblobsize>(blob.size());
      const auto bufferSize = blobSize + sizeof(fcblobsize);
      
      auto& vec = std::get<CachedValue::VEC>(it->second.value);
      vec.type = FBT_BLOB;
      vec.extract = extractBlob;
      vec.data.resize(bufferSize);

      auto buffer = vec.data.data();

      std::memcpy(buffer, &blobSize, sizeof(fcblobsize));
      std::memcpy(buffer+sizeof(fcblobsize), blob.data(), blob.size());
    }


  private:
    Map m_map;
  };

}