#pragma once

#include <ankerl/unordered_dense.h>
#include <fc/KvCommon.hpp>
#include <fc/Memory.hpp>
#include <plog/Log.h>


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
    

    bool init()
    {
      return m_fixed.init();
    }


    template<bool IsSet, flexbuffers::Type FlexT, typename ValueT>
    bool setOrAdd (const CachedKey& key, const ValueT& v) noexcept
    {
      try
      {
        if constexpr (FlexT == flexbuffers::Type::FBT_STRING || FlexT == flexbuffers::Type::FBT_VECTOR)
        {
          //doVariableSetAdd<IsSet, FlexT, ValueT>(key, v);
        }
        else
        {
          doFixedSetAdd<IsSet, FlexT>(key, v);
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

            cachedValue.extract(m_fixed, fb, pKey, cachedValue);
          }
        }      
      });
    }


    void remove (const KeyVector& keys)
    {
      for (const auto& key : keys)
      {
        if (auto it = m_map.find(key->str()); it != m_map.end())
        {
          //it->second.
        }
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
    void doFixedSetAdd(const CachedKey& key, const ValueT value)
    {
      ExtractF extract{nullptr};

      if constexpr (FlexT == flexbuffers::Type::FBT_INT)
        extract = extractInt;
      else if constexpr (FlexT == flexbuffers::Type::FBT_UINT)
        extract = extractUInt;
      else if constexpr (FlexT == flexbuffers::Type::FBT_FLOAT)
        extract = extractFloat;
      else if constexpr (FlexT == flexbuffers::Type::FBT_BOOL)
        extract = extractBool;
      else
        static_assert("FlexBuffers::Type not supported for fixed type");


      if (const auto it = m_map.find(key) ; it == m_map.end())
      {
        const auto [block, pos] = m_fixed.write<ValueT>(value);
        m_map.try_emplace(key, block, extract, pos, sizeof(ValueT));
      }
      else
      {
        // set overwrites existing, but add does not
        if constexpr (IsSet)
        {
          auto& cv = it->second;

          // buffer may change if new value occupies more space than existing value
          // i.e. bool value overwritten to an int
          const auto [block, pos] = m_fixed.overwrite<ValueT>(cv.block, cv.blockPos, cv.size, value);
          cv.block = block;
          cv.blockPos = pos;
          cv.extract = extract; // possibly different extract function if value type different
        }        
      }
    }


    template<bool IsSet, flexbuffers::Type FlexT, typename ValueT>
    void doVariableSetAdd(const CachedKey& key,  const ValueT& v)
    {
      /*
      ValueExtractF extract{nullptr};
      Buffer buff {.size = v.size(), .buffer = std::make_shared<uint8_t[]>(v.size())};

      if constexpr (FlexT == flexbuffers::Type::FBT_STRING)
      {
        extract = extractString;
        std::memcpy(buff.buffer.get(), v.c_str(), buff.size);
      }
      else if constexpr (FlexT == flexbuffers::Type::FBT_VECTOR)
      {
        extract = extractVector;

        const flexbuffers::Vector& vec = v;
        for (std::size_t i = 0 ; i < vec.size() ; ++i)
        {
          switch (vec[i].GetType())
          {
            using enum flexbuffers::Type;

            case FBT_INT:
            
              break;
            
            default:
              break;
          }
        }
      }
      else
        static_assert("Unsupported FlexBuffer::Type");

      if constexpr (IsSet)
      {
        m_map.insert_or_assign(key, CachedValue{buff, extract});
      }
      else
        m_map.try_emplace(key, buff, extract);
      */
    }
    
    
    static void extractInt(FixedMemory& fm, FlexBuilder& fb, const char * key, const CachedValue& cv)
    {
      const auto v = fm.get<int>(cv.block, cv.blockPos);
      fb.Add(key, v);
    }

    
    static void extractUInt(FixedMemory& fm, FlexBuilder& fb, const char * key, const CachedValue& cv)
    {
      const auto v = fm.get<unsigned int>(cv.block, cv.blockPos);
      fb.UInt(key, v);
    }

    
    static void extractFloat(FixedMemory& fm, FlexBuilder& fb, const char * key, const CachedValue& cv)
    {
      const auto v = fm.get<float>(cv.block, cv.blockPos);
      fb.Float(key, v);
    }

    
    static void extractBool(FixedMemory& fm, FlexBuilder& fb, const char * key, const CachedValue& cv)
    {
      const auto v = fm.get<bool>(cv.block, cv.blockPos);
      fb.Bool(key, v);
    }

    // static void extractString(const char * key, const CachedValue& cv, FlexBuilder& fb)
    // {
    //   const auto& varVariant = std::get<CachedValue::VAR>(cv.value);
    //   fb.String(key, flexbuffers::String(varVariant.buffer.get(), sizeof(char), varVariant.size));
    // }

    // static void extractVector(const char * key, const CachedValue& cv, FlexBuilder& fb)
    // {
    //   const auto& varVariant = std::get<CachedValue::VAR>(cv.value);
    //   fb.Vector<uint8_t>(key, varVariant.buffer.get(), varVariant.size);
    // }

  private:
    Map m_map;
    FixedMemory m_fixed;
  };

}