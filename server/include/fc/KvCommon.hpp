#pragma once

#include <variant>
#include <fc/FlatBuffers.hpp>
#include <fc/Memory.hpp>


namespace fc
{  
  struct CachedValue;


  using KeyVector = flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>;  
  using CachedKey = std::string;
  using ExtractF = void (*)(FixedMemory&, FlexBuilder&, const char *, const CachedValue&);

  
  struct CachedValue
  {
    Block * block;
    ExtractF extract;
    std::size_t blockPos; // position in the Block's position
    std::uint16_t size; // only required for variable sizes
  };
}