#include <fc/Map.hpp>


namespace fc
{
  template<typename V>
  static void toTypedVector (FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const auto& values = std::get<V>(vv.vec);

    if constexpr (std::is_same_v<V, StringVector>)
    {
      fb.TypedVector(key, [&values, &fb]()
      {
        for (const auto& value : values)
          fb.String(value.data());
      }); 
    }
    else
    {
      fb.TypedVector(key, [&values, &fb]
      {
        for (const auto val : values)
          fb.Add(val);
      });
    }
  }

  
  void CacheMap::extractInt(FlexBuilder& fb, const char * key, const FixedValue& fv)
  {
    fb.Add(key, std::get<std::int64_t>(fv.value));
  }

  
  void CacheMap::extractUInt(FlexBuilder& fb, const char * key, const FixedValue& fv)
  {
    fb.Add(key, std::get<std::uint64_t>(fv.value));
  }

  
  void CacheMap::extractFloat(FlexBuilder& fb, const char * key, const FixedValue& fv)
  {
    fb.Add(key, std::get<float>(fv.value));
  }

  
  void CacheMap::extractBool(FlexBuilder& fb, const char * key, const FixedValue& fv)
  {
    fb.Add(key, std::get<bool>(fv.value));
  }


  void CacheMap::extractBlob(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const auto& vec = std::get<BlobVector>(vv.vec);
    fb.Blob(key, vec.data(), vec.size());
  }
  
  
  void CacheMap::extractCharV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const auto& charVector = std::get<CharVector>(vv.vec);
    fb.String(key, charVector.data());
  }


  void CacheMap::extractStringV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<StringVector>(fb, key, vv);
  }


  void CacheMap::extractIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<IntVector>(fb, key, vv);
  }


  void CacheMap::extractUIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<UIntVector>(fb, key, vv);
  }


  void CacheMap::extractFloatV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<FloatVector>(fb, key, vv);
  }


  void CacheMap::extractBoolV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<BoolVector>(fb, key, vv);
  }
}