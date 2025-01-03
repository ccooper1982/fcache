#pragma once

#include <variant>
#include <fc/FlatBuffers.hpp>


namespace fc
{  

  using KeyVector = flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>;  
  using ValueVariant = std::variant<std::string, int64_t, uint64_t, double, bool>;
  using ValueExtractF = void (*)(const char *, const ValueVariant&, FlexBuilder&);

  // This struct is 48 bytes which is larger than I want. The 
  // std::string in ValueVariant is 32 bytes, but it will do
  // for now until memory slabs are implemented
  struct CachedValue
  {
    static const std::size_t GET_STR = 0;
    static const std::size_t GET_INT = 1;
    static const std::size_t GET_UINT = 2;
    static const std::size_t GET_DBL = 3;
    static const std::size_t GET_BOOL = 4;

    CachedValue(const ValueVariant vv, const ValueExtractF f) : value(vv), extract(f)
    {
    }

    ValueVariant value;
    ValueExtractF extract;
  };

  using CachedKey = std::string;
}