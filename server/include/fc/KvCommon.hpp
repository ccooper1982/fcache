#pragma once

#include <variant>
#include <fc/FlatBuffers.hpp>
#include <fc/Memory.hpp>


namespace fc
{  
  struct CachedValue;

  using KeyVector = flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>;  
  using CachedKey = std::string;
  using ExtractFixedF = void (*)(FlexBuilder&, const char * key, const CachedValue&);

  // This will do for now.
  struct CachedValue
  { 
    inline static const std::uint8_t MEM_FIXED = 0;
    inline static const std::uint8_t MEM_VAR   = 1;

    std::variant<FixedMemory*, VariedMemory*> mem;
    ExtractFixedF extractFixed;
    BlockView bv;
    std::uint8_t memType;    
  };

}