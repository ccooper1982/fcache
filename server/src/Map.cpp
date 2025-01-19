#include <fc/Map.hpp>


namespace fc
{
  template<typename T>
  static void toTypedVector(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    fb.TypedVector(key, [&data = vv.data, &fb]
    {
      for (std::size_t i = 0 ; i < data.size() ; i += sizeof(T))
      {
        T v{};
        std::memcpy(&v, data.data()+i, sizeof(T));
        fb.Add(v);
      }
    });
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
    const auto buffer = vv.data.data();

    std::uint32_t size{0};
    std::memcpy(&size, buffer, sizeof(std::uint32_t));

    fb.Blob(key, buffer+sizeof(std::uint32_t), size);
  }
  
  
  void CacheMap::extractString(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const char * buffer = reinterpret_cast<const char*>(vv.data.data());
    const std::string_view sv{buffer};
    fb.String(key, sv.data());
  }


  void CacheMap::extractStringV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const char * buffer = reinterpret_cast<const char*>(vv.data.data());

    fb.TypedVector(key, [&fb, buffer, size = vv.data.size()]() mutable
    {
      for (std::size_t i = 0 ; i < size ; )
      {
        const std::string_view sv{buffer+i};

        fb.String(sv.data());

        i += sv.size()+1; // length of string, +1 to skip '\0'
      }
    });
  }


  void CacheMap::extractIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<int64_t>(fb, key, vv);
  }


  void CacheMap::extractUIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<uint64_t>(fb, key, vv);
  }


  void CacheMap::extractFloatV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<float>(fb, key, vv);
  }


  void CacheMap::extractBoolV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<bool>(fb, key, vv);
  }
}