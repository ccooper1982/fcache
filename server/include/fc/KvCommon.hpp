#pragma once

#include <variant>
#include <fc/FlatBuffers.hpp>
#include <fc/Memory.hpp>


namespace fc
{  
  using KeyVector = flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>;  
  using CachedKey = std::string;
  using ExtractF = void (*)(FixedMemory&, FlexBuilder&, const char *, char *);

  struct CachedValue
  {
    char * buffer;
    ExtractF extract;
    std::uint16_t size; // only required for variable sizes
  };
}