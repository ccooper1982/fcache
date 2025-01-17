#pragma once

#include <ankerl/unordered_dense.h>
#include <fc/KvCommon.hpp>
#include <plog/Log.h>


namespace fc
{
  class CacheMap
  {
    using Map = ankerl::unordered_dense::pmr::map<std::pmr::string, CachedValue>;
    using CacheMapIterator = Map::iterator;
    using CacheMapConstIterator = Map::const_iterator;
    using enum FlexType;


  public:
    CacheMap() : m_map(std::pmr::polymorphic_allocator {fc::MapMemory::getPool()})
    {

    }

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

        const std::pmr::string pmrKey{key, MapMemory::getPool()};

        // set and add both insert if key not exist
        if (const auto it = m_map.find(pmrKey) ; it == m_map.end())
        {
          FixedValue fv {.value = value, .extract = extract};
          m_map.try_emplace(pmrKey, fv, CachedValue::FIXED);
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
        const std::pmr::string pmrKey{key, MapMemory::getPool()};
        
        if (const auto it = m_map.find(pmrKey) ; it == m_map.end())
        {
          makeVectorValue<FlexT>(pmrKey, v);
        }
        else if (IsSet)
        {
          PLOGE << "TODO";
          //it->second.value
          //it->second.valueType = CachedValue::VEC;
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
        const std::pmr::string pmrKey{key, MapMemory::getPool()};
        
        if (const auto it = m_map.find(pmrKey) ; it == m_map.end())
        {
          m_map.try_emplace(pmrKey,
                            VectorValue {.vec = String{str, VectorMemory::getPool()}, .extract = extractString},
                            CachedValue::VEC);
        }
        else if (IsSet)
        {
          PLOGE << "TODO";
          //it->second.value
          //it->second.valueType = CachedValue::VEC;
        }

        //insertVectorValue<IsSet>(key, makeVectorValue<FlexT>(v));
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
        const std::pmr::string pmrKey{key, MapMemory::getPool()};
        
        if (const auto it = m_map.find(pmrKey) ; it == m_map.end())
        {
          auto [itEmplaced, emplaced] = m_map.try_emplace(pmrKey,
                                                  VectorValue {.vec = BlobVector{VectorMemory::getPool()}, .extract = extractBlob},
                                                  CachedValue::VEC);

          auto& vv = std::get<VectorValue>(itEmplaced->second.value);
          auto& dest = std::get<BlobVector>(vv.vec);
          dest.resize(blob.size());
          std::memcpy(dest.data(), blob.data(), blob.size());
        }
        else if (IsSet)
        {
          PLOGE << "TODO";
          //it->second.value
          //it->second.valueType = CachedValue::VEC;
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
          const std::pmr::string pmrKey{key->str(), MapMemory::getPool()};

          if (const auto& it = m_map.find(pmrKey); it != m_map.cend())
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
        const std::pmr::string pmrKey{key->str(), MapMemory::getPool()};
        m_map.erase(pmrKey);
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
        const auto& stdString = key->str();
        const std::pmr::string pmrKey{key->str(), MapMemory::getPool()};

        if (m_map.contains(pmrKey))
          keysExist.emplace_back(stdString);
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
    static void extractString(FlexBuilder& fb, const char * key, const VectorValue& vv);
    

    VectorValue makeBlobVectorValue (const std::uint8_t * data, const std::size_t size)
    {
      auto vec = createFcVectorSized<uint8_t>(size);
      std::memcpy(vec.data(), data, size);
      return VectorValue {.vec = std::move(vec), .extract = extractBlob};
    }


    template<typename ElementT, typename VectorT>
    void copyToMap (const flexbuffers::TypedVector& source, VectorValue& vv) 
    {
      auto& dest = std::get<VectorT>(vv.vec);
      dest.resize(source.size());

      for (std::size_t i = 0 ; i < source.size() ; ++i)
      {
        if constexpr (std::is_same_v<ElementT, std::pmr::string>)
          dest[i].assign(source[i].AsString().c_str());
        else
          dest[i] = source[i].template As<ElementT>();
      }
    }


    template<FlexType FlexT>
    void makeVectorValue (const std::pmr::string& key, const flexbuffers::TypedVector& source)
    {
      // TODO this can be tidied

      if constexpr (FlexT == FBT_VECTOR_INT)
      {
        auto [it, emplaced] = m_map.try_emplace(  key,
                                                  VectorValue {.vec = IntVector{VectorMemory::getPool()}, .extract = extractIntV},
                                                  CachedValue::VEC);

        auto& vv = std::get<VectorValue>(it->second.value);
        copyToMap<int64_t, IntVector>(source, vv);
      }
      else if constexpr (FlexT == FBT_VECTOR_UINT)
      {
        auto [it, emplaced] = m_map.try_emplace(  key,
                                                  VectorValue {.vec = UIntVector{VectorMemory::getPool()}, .extract = extractUIntV},
                                                  CachedValue::VEC);

        auto& vv = std::get<VectorValue>(it->second.value);
        copyToMap<uint64_t, UIntVector>(source, vv);
      }
      else if constexpr (FlexT == FBT_VECTOR_FLOAT)
      {
        auto [it, emplaced] = m_map.try_emplace(  key,
                                                  VectorValue {.vec = FloatVector{VectorMemory::getPool()}, .extract = extractFloatV},
                                                  CachedValue::VEC);

        auto& vv = std::get<VectorValue>(it->second.value);
        copyToMap<float, FloatVector>(source, vv);
      }
      else if constexpr (FlexT == FBT_VECTOR_BOOL)
      {
        auto [it, emplaced] = m_map.try_emplace(  key,
                                                  VectorValue {.vec = BoolVector(VectorMemory::getPool()), .extract = extractBoolV},
                                                  CachedValue::VEC);

        auto& vv = std::get<VectorValue>(it->second.value);
        copyToMap<bool, BoolVector>(source, vv);
      }
      else if constexpr (FlexT == FBT_VECTOR_KEY) // vector of strings
      {
        auto [it, emplaced] = m_map.try_emplace(  key,
                                                  VectorValue {.vec = StringVector{VectorMemory::getPool()}, .extract = extractStringV},
                                                  CachedValue::VEC);

        auto& vv = std::get<VectorValue>(it->second.value);
        copyToMap<std::pmr::string, StringVector>(source, vv);
      }
      else
        static_assert(false, "Unsupported vector type");
    }


    // template<typename ElementT>
    // fc::Vector<ElementT> toFcVector(const flexbuffers::TypedVector& source)
    // {
    //   if constexpr (std::is_same_v<ElementT, std::pmr::string>)
    //   {
    //     auto dest = createFcVectorReserved<ElementT>(source.size());
    //     for (std::size_t i = 0 ; i < source.size() ; ++i)
    //       dest.emplace_back(source[i].AsString().c_str());
      
    //     return dest;
    //   }
    //   else
    //   {
    //     auto dest = createFcVectorSized<ElementT>(source.size());
    //     for (std::size_t i = 0 ; i < source.size() ; ++i)
    //       dest[i] = source[i].template As<ElementT>();
      
    //     return dest;
    //   }
    // }

  private:
    Map m_map;
  };

}