#include <fc/Map.hpp>


namespace fc
{
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


  void CacheMap::extractStringV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    fb.TypedVector(key, [&vv, &fb]()
    {
      const auto& strings = std::get<StringVector>(vv.vec);
      for (const auto& s : strings)
        fb.String(s.data());
    });
  }


  // TODO should these not all be TypedVector?

  void CacheMap::extractIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const auto& vec = std::get<IntVector>(vv.vec);
    fb.Vector(key, vec.data(), vec.size());
  }


  void CacheMap::extractUIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const auto& vec = std::get<UIntVector>(vv.vec);
    fb.Vector(key, vec.data(), vec.size());
  }


  void CacheMap::extractFloatV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const auto& vec = std::get<FloatVector>(vv.vec);
    fb.Vector(key, vec.data(), vec.size());
  }


  void CacheMap::extractBoolV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    fb.Vector(key, [&vv, &fb]
    {
      const auto& bools = std::get<BoolVector>(vv.vec);
      for (const auto& b : bools)
        fb.Add(b);
    });
  }


  void CacheMap::extractCharV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const auto& charVector = std::get<CharVector>(vv.vec);
    fb.Key(key);
    fb.String(charVector.data(), charVector.size());
  }
}