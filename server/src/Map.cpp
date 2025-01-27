#include <fc/Map.hpp>
#include <fc/Common.hpp>

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
    fb.Add(key, std::get<fcint>(fv.value));
  }

  
  void CacheMap::extractUInt(FlexBuilder& fb, const char * key, const FixedValue& fv)
  {
    fb.Add(key, std::get<fcuint>(fv.value));
  }

  
  void CacheMap::extractFloat(FlexBuilder& fb, const char * key, const FixedValue& fv)
  {
    fb.Add(key, std::get<fcfloat>(fv.value));
  }

  
  void CacheMap::extractBool(FlexBuilder& fb, const char * key, const FixedValue& fv)
  {
    fb.Add(key, std::get<fcbool>(fv.value));
  }


  void CacheMap::extractBlob(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    const auto buffer = vv.data.data();

    fcblobsize size{0};
    std::memcpy(&size, buffer, sizeof(fcblobsize));

    fb.Blob(key, buffer+sizeof(fcblobsize), size);
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
    toTypedVector<fcint>(fb, key, vv);
  }


  void CacheMap::extractUIntV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<fcuint>(fb, key, vv);
  }


  void CacheMap::extractFloatV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<fcfloat>(fb, key, vv);
  }


  void CacheMap::extractBoolV(FlexBuilder& fb, const char * key, const VectorValue& vv)
  {
    toTypedVector<fcbool>(fb, key, vv);
  }
}