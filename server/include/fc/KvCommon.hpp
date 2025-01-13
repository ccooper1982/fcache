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


  // Temporary solution until Memory.hpp is used:
  //  - store vector<T> (including strings) in memory blocks
  //  - a block is pre-allocated memory
  //  - write values to a memory block, 'flattening' the vector to contigious bytes
  // VectorValue may become:
  //  struct VariedValue
  //  {
  //    BlockView view; // contains BlockId (which block),
  //                    //          BlockPos (position in the block)
  //                    //          std::size_t (size of data)
  //                    //          FlexType (type of data, required when creating FlexBuffer response)
  //    Memory * mem;   // may have a string specific Memory handler
  //  }
  //
  // VariedValue - the value is not fixed size (unlike int, float, bool, etc)
  //             - contains everything required to extract the value.
  struct VectorValue
  {
    using IntVector = std::vector<std::int64_t>;
    using UIntVector = std::vector<std::uint64_t>;
    using FloatVector = std::vector<float>;
    using BoolVector = std::vector<bool>;
    using CharVector = std::vector<char>; // strings are a vector of chars
    using StringVector = std::vector<std::string>;  // TODO, perhaps std::vector<CharVector>
    using BlobVector = std::vector<uint8_t>;

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
}