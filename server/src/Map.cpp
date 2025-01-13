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
    fb.Blob(key, std::get<VectorValue::BlobVector>(vv.vec));
  }


  void CacheMap::extractString(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    fb.Vector(key, [&vv, &fb]()
    {
      const auto& strings = std::get<VectorValue::StringVector>(vv.vec);
      for (const auto& s : strings)
        fb.Add(s);
    });
  }


  void CacheMap::extractIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    fb.Add(key, std::get<VectorValue::IntVector>(vv.vec));
  }


  void CacheMap::extractUIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    fb.Add(key, std::get<VectorValue::UIntVector>(vv.vec));
  }


  void CacheMap::extractFloatV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    fb.Add(key, std::get<VectorValue::FloatVector>(vv.vec));
  }


  void CacheMap::extractBoolV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    fb.Vector(key, [&vv, &fb]
    {
      const auto& bools = std::get<VectorValue::BoolVector>(vv.vec);
      for (const auto& b : bools)
        fb.Add(b);
    });
  }


  void CacheMap::extractCharV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const auto& charVector = std::get<VectorValue::CharVector>(vv.vec);
    fb.Key(key);
    fb.String(charVector.data(), charVector.size());
  }
}