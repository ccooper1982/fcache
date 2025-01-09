#pragma once

#include <variant>
#include <fc/FlatBuffers.hpp>
#include <fc/Memory.hpp>


namespace fc
{ 
  struct FixedValue;
  struct VectorValue;


  using CachedKey = std::string;
  using KeyVector = flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>;  
  using ExtractFixedF = void (*)(FlexBuilder&, const char * key, const FixedValue&);
  using ExtractVectorF = void (*)(FlexBuilder&, const char * key, const VectorValue&);


  struct FixedValue
  {
    std::variant<std::int64_t, std::uint64_t, float, bool> value;
    ExtractFixedF extract;
  };


  struct VectorValue
  {
    using IntVector = std::vector<std::int64_t>;
    using UIntVector = std::vector<std::uint64_t>;
    using FloatVector = std::vector<float>;

    std::variant<IntVector, UIntVector, FloatVector> vec;
    ExtractVectorF extract;
  };


  struct CachedValue
  {
    inline static const std::uint8_t FIXED = 0;
    inline static const std::uint8_t VEC   = 1;

    std::variant<FixedValue, VectorValue> value;
    std::uint8_t valueType;    
  };
}