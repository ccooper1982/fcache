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


  template<typename T>
  using Vector = std::pmr::vector<T>;

  using IntVector = Vector<std::int64_t>;
  using UIntVector = Vector<std::uint64_t>;
  using FloatVector = Vector<float>;
  using BoolVector = Vector<bool>;
  using CharVector = Vector<char>; 
  using StringVector = Vector<std::pmr::string>;  // TODO, perhaps Vector<CharVector>
  using BlobVector = Vector<uint8_t>;

  
  struct FixedValue
  {
    std::variant<std::int64_t, std::uint64_t, float, bool> value;
    ExtractFixedF extract;
  };

 
  struct VectorValue
  {
    std::variant< IntVector, UIntVector, FloatVector,
                  CharVector, StringVector, BoolVector,
                  BlobVector> vec;
    ExtractVectorF extract;
  };


  struct CachedValue
  {
    inline static const std::uint8_t FIXED = 0;
    inline static const std::uint8_t VEC   = 1;

    std::variant<FixedValue, VectorValue> value;
    std::uint8_t valueType;
  };

   
  template<typename ElementT>
  fc::Vector<ElementT> createFcVectorSized(const std::size_t size)  
  {
    fc::Vector<ElementT> dest(Memory::getPool());
    dest.resize(size);
    return dest;
  }

  template<typename ElementT>
  fc::Vector<ElementT> createFcVectorReserved(const std::size_t size)  
  {
    fc::Vector<ElementT> dest(Memory::getPool());
    dest.reserve(size);
    return dest;
  }
}