#pragma once

#include <ankerl/unordered_dense.h>
#include <fc/KvCommon.hpp>
#include <plog/Log.h>


namespace fc
{
  class CacheMap
  {
    using Map = ankerl::unordered_dense::segmented_map<CachedKey, CachedValue>;
    using CacheMapIterator = Map::iterator;
    using CacheMapConstIterator = Map::const_iterator;
    using enum FlexType;

  public:
    
    CacheMap() = default;

    CacheMap& operator=(CacheMap&&) = default; // required by Map::erase()
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


        // set and add both insert if key not exist
        if (const auto it = m_map.find(key) ; it == m_map.end())
        {
          FixedValue fv {.value = value, .extract = extract};
          m_map.try_emplace(key, fv, CachedValue::FIXED);
        }
        else if constexpr (IsSet)
        {
          // but only set overwrites existing
          it->second.value = FixedValue {.value = value, .extract = extract};
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
        insertVectorValue<IsSet>(key, makeVectorValue<FlexT>(v));
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
        // a string is handled as a vector<char>
        insertVectorValue<IsSet>(key, makeCharVectorValue(str));
      }
      catch(const std::exception& e)
      {
        PLOGE << __FUNCTION__ << ": " << e.what();
        return false;
      }
      return true;
    }


    template<bool IsSet>
    bool setOrAdd (const CachedKey& key, const flexbuffers::Blob& blob)
    {
      try
      {
        insertVectorValue<IsSet>(key, makeBlobVectorValue(blob.data(), blob.size()));
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


    void remove (const KeyVector& keys)
    {
      for (const auto& key : keys)
      {
        m_map.erase(key->str());
      }
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

    
    flatbuffers::Offset<KeyVector> contains (FlatBuilder& fb, const KeyVector& keys) const
    {
      // TODO change this response to use a flexbuffer, responding with a TypedVector (FBT_VECTOR_KEY)

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
    static void extractCharV(FlexBuilder& fb, const char * key, const VectorValue& vv);
    


    template<bool IsSet>
    void insertVectorValue (const CachedKey& key, VectorValue vv)
    {
      if (const auto it = m_map.find(key) ; it == m_map.end())
      {
        m_map.try_emplace(key, vv, CachedValue::VEC);
      }
      else if constexpr (IsSet)
      {
        it->second.value = vv ;
        it->second.valueType = CachedValue::VEC;
      }
    }
    

    VectorValue makeCharVectorValue (const std::string_view str)
    {
      // must add the null terminator here or when writing to response buffer:
      //  originally the '\0' was omitted. But when running through 
      //  valgrind, it complained of invalid read in CacheMap::extractCharV()
      //  It was written to flexbuffer with:  fb.String(vec.data(), vec.size())
      //  but that String() calls CreateBlock() and assumes there's a trailing
      //  byte, which must be the `\0`.
      // or use std::pmr::string
      auto vec = createFcVectorSized<char>(str.size()+1); 
      *(vec.rbegin()) = '\0';
      std::memcpy(vec.data(), str.data(), vec.size());
      return VectorValue {.vec = std::move(vec), .extract = extractCharV};
    }


    VectorValue makeBlobVectorValue (const std::uint8_t * data, const std::size_t size)
    {
      auto vec = createFcVectorSized<uint8_t>(size);
      std::memcpy(vec.data(), data, size);
      return VectorValue {.vec = std::move(vec), .extract = extractBlob};
    }


    template<FlexType FlexT>
    VectorValue makeVectorValue (const flexbuffers::TypedVector& vector)
    {
      if constexpr (FlexT == FBT_VECTOR_INT)
        return VectorValue {.vec = toFcVector<std::int64_t>(vector), .extract = extractIntV};
      else if constexpr (FlexT == FBT_VECTOR_UINT)
        return VectorValue {.vec = toFcVector<std::uint64_t>(vector), .extract = extractUIntV};
      else if constexpr (FlexT == FBT_VECTOR_FLOAT)
        return VectorValue {.vec = toFcVector<float>(vector), .extract = extractFloatV};
      else if constexpr (FlexT == FBT_VECTOR_BOOL)
        return VectorValue {.vec = toFcVector<bool>(vector), .extract = extractBoolV};
      else if constexpr (FlexT == FBT_VECTOR_KEY)
        return VectorValue {.vec = toFcVector<std::pmr::string>(vector), .extract = extractStringV};
    }


    template<typename ElementT>
    fc::Vector<ElementT> toFcVector(const flexbuffers::TypedVector& source)
    {
      if constexpr (std::is_same_v<ElementT, std::pmr::string>)
      {
        auto dest = createFcVectorReserved<ElementT>(source.size());
        for (std::size_t i = 0 ; i < source.size() ; ++i)
          dest.emplace_back(source[i].AsString().c_str());
      
        return dest;
      }
      else
      {
        auto dest = createFcVectorSized<ElementT>(source.size());
        for (std::size_t i = 0 ; i < source.size() ; ++i)
          dest[i] = source[i].template As<ElementT>();
      
        return dest;
      }
    }

  private:
    Map m_map;
  };

}