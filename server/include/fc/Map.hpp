#pragma once

#include <ankerl/unordered_dense.h>
#include <fc/KvCommon.hpp>
#include <fc/Memory.hpp>
#include <plog/Log.h>
#include <ranges>


namespace fc
{
  class CacheMap
  {
    using Map = ankerl::unordered_dense::segmented_map<CachedKey, CachedValue>;
    using CacheMapIterator = Map::iterator;
    using CacheMapConstIterator = Map::const_iterator;
    using enum flexbuffers::Type;

  public:
    
    CacheMap& operator=(CacheMap&&) = default; // required by Map::erase()
    CacheMap(CacheMap&&) = default;

    CacheMap& operator=(const CacheMap&) = delete;   
    CacheMap(CacheMap&) = delete;

    
    CacheMap (const std::size_t buckets = 0) : m_map(buckets) 
    {
    }
    

    bool init()
    {
      return true;
    }


    template<bool IsSet, flexbuffers::Type FlexT, typename ValueT>
    bool setOrAdd (const BufferVector& vecBuffer, const CachedKey& key, const ValueT& v) noexcept
    {
      try
      {
        if constexpr (std::is_integral_v<ValueT> || std::is_same_v<ValueT, float>)
        {
          doFixedSetAdd<IsSet, FlexT>(key, v);
        } 
        else if constexpr (std::is_same_v<ValueT, std::string>)
        {
          doStringSetAdd(key, v);          
        }  
        else if constexpr (std::is_same_v<ValueT, flexbuffers::TypedVector>)
        {
          doVectorSetAdd<IsSet, FlexT>(vecBuffer, key, v);
        }

        return true;
      }
      catch(const std::exception& ex)
      {
        PLOGE << ex.what();
      }
      
      return false;
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
      for (const auto key : keys)
      {
        m_map.erase(key->str());
      }
    };

    
    bool clear() noexcept
    {
      try
      {
        m_map.replace(Map::value_container_type{});
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

    template<bool IsSet, flexbuffers::Type FlexT, typename ValueT>
    void doFixedSetAdd(const CachedKey& key, const ValueT value)  requires(std::is_integral_v<ValueT> || std::is_same_v<ValueT, float>)
    {
      ExtractFixedF extract{nullptr};

      if constexpr (FlexT == flexbuffers::Type::FBT_INT)
        extract = extractInt;
      else if constexpr (FlexT == flexbuffers::Type::FBT_UINT)
        extract = extractUInt;
      else if constexpr (FlexT == flexbuffers::Type::FBT_FLOAT)
        extract = extractFloat;
      else if constexpr (FlexT == flexbuffers::Type::FBT_BOOL)
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
    }


    template<bool IsSet, flexbuffers::Type FlexT>
    void doVectorSetAdd(const BufferVector& vecBuffer, const CachedKey& key, const flexbuffers::TypedVector& v)
    { 
      VectorValue vv = makeVectorValue<FlexT>(v);

      if (const auto it = m_map.find(key) ; it == m_map.end())
      {
        m_map.try_emplace(key, vv, CachedValue::VEC);
      }
      else if constexpr (IsSet)
      {
        // TODO (current value is VectorValue AND
        //      v.size() <= existing size() AND
        //      same type): 
        //      then don't need to make a new VectorValue, can just copy over, and possibly shrink
        it->second.value = vv ;
        it->second.valueType = CachedValue::VEC;
      }
    }


    void doStringSetAdd(const CachedKey& key,  const std::string& v)
    {
      // TODO
    }
   

    template<flexbuffers::Type FlexT>
    VectorValue makeVectorValue (const flexbuffers::TypedVector& vector)
    {
      if constexpr (FlexT == FBT_VECTOR_INT)
        return VectorValue {.vec = copyVector<std::int64_t>(vector), .extract = extractIntV};
      else if constexpr (FlexT == FBT_VECTOR_UINT)
        return VectorValue {.vec = copyVector<std::uint64_t>(vector), .extract = extractUIntV};
      else if constexpr (FlexT == FBT_VECTOR_FLOAT)
        return VectorValue {.vec = copyVector<float>(vector), .extract = extractFloatV};
    }


    template<typename ElementT>
    std::vector<ElementT> copyVector(const flexbuffers::TypedVector& source)
    {
      std::vector<ElementT> dest;
      dest.resize(source.size());

      for (std::size_t i = 0 ; i < source.size() ; ++i)
          dest[i] = source[i].As<ElementT>();

      return dest;
    }

    
    static void extractInt(FlexBuilder& fb, const char * key, const FixedValue& fv)
    {
      fb.Add(key, std::get<std::int64_t>(fv.value));
    }

    
    static void extractUInt(FlexBuilder& fb, const char * key, const FixedValue& fv)
    {
      fb.Add(key, std::get<std::uint64_t>(fv.value));
    }

    
    static void extractFloat(FlexBuilder& fb, const char * key, const FixedValue& fv)
    {
      fb.Add(key, std::get<float>(fv.value));
    }

    
    static void extractBool(FlexBuilder& fb, const char * key, const FixedValue& fv)
    {
      fb.Add(key, std::get<bool>(fv.value));
    }


    static void extractIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
    {
      fb.Add(key, std::get<VectorValue::IntVector>(vv.vec));
    }

    static void extractUIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
    {
      fb.Add(key, std::get<VectorValue::UIntVector>(vv.vec));
    }

    static void extractFloatV(FlexBuilder& fb, const char * key, const VectorValue& vv)
    {
      fb.Add(key, std::get<VectorValue::FloatVector>(vv.vec));
    }


    // static void extractString(const char * key, const CachedValue& cv, FlexBuilder& fb)
    // {
    //   const auto& varVariant = std::get<CachedValue::VAR>(cv.value);
    //   fb.String(key, flexbuffers::String(varVariant.buffer.get(), sizeof(char), varVariant.size));
    // }


  private:
    Map m_map;
    //FixedMemory * m_fixed;
  };

}