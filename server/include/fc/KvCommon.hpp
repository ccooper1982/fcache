#pragma once

#include <variant>
#include <fc/FlatBuffers.hpp>
#include <fc/Memory.hpp>


namespace fc
{ 
  struct FixedValue;

  using CachedKey = std::string;
  using KeyVector = flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>;  
  using ExtractFixedF = void (*)(FlexBuilder&, const char * key, const FixedValue&);


  struct FixedValue
  {
    std::variant<std::int64_t, std::uint64_t, float, bool> value;
    ExtractFixedF extract;
  };

  struct VectorValue
  {
    using IntVector = std::vector<std::int64_t>;
    using UIntVector = std::vector<std::uint64_t>;

    std::variant<IntVector, UIntVector> vec;
    flexbuffers::Type vecType;  // TODO need a key for std::get<>(vec)
  };


  struct CachedValue
  {
    inline static const std::uint8_t FIXED = 0;
    inline static const std::uint8_t VEC   = 1;

    std::variant<FixedValue, VectorValue> value;
    std::uint8_t valueType;
  };
}