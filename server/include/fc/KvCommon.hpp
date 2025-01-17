#pragma once

#include <variant>
#include <fc/FlatBuffers.hpp>
#include <fc/Memory.hpp>


namespace fc
{ 
  struct FixedValue;
  struct VectorValue;


  using CachedKey = std::string;  // TODO change to std::pmr::string, requires some func signatures changed
  using KeyVector = flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>;  
  using ExtractFixedF = void (*)(FlexBuilder&, const char * key, const FixedValue&);
  using ExtractVectorF = void (*)(FlexBuilder&, const char * key, const VectorValue&);


  template<typename T>
  using Vector = std::pmr::vector<T>;

  using IntVector = Vector<std::int64_t>;
  using UIntVector = Vector<std::uint64_t>;
  using FloatVector = Vector<float>;
  using BoolVector = Vector<bool>;
  using String = std::pmr::string; 
  using StringVector = Vector<std::pmr::string>;
  using BlobVector = Vector<uint8_t>;

  
  struct FixedValue
  {
    std::variant<std::int64_t, std::uint64_t, float, bool> value;
    ExtractFixedF extract;
  };

 
  struct VectorValue
  {
    std::variant< IntVector, UIntVector, FloatVector,
                  String, StringVector, BoolVector,
                  BlobVector> vec;
    ExtractVectorF extract;
  };

  /*
  Investigate treating all data as a bunch of bytes (BoB):

  struct CachedValue
  {
    // FixedValue as it is now

    // VectorValue doesn't differentiate types, it just stores a BoB.
    // problem is a vector strings, need the length of string
    // maybe: [str_len][...string...][str_len][...string...]
    struct VectorValue
    {
      std::vector<uint8_t> data;
      FlexType type;
    };

    std::variant<FixedValue, VectorValue> value;
  };
  */

  struct CachedValue
  {
    inline static const std::uint8_t FIXED = 0;
    inline static const std::uint8_t VEC   = 1;

    using allocator_type = std::pmr::polymorphic_allocator<>;

    CachedValue() = default;
    CachedValue(const CachedValue&) noexcept = default;
    CachedValue(CachedValue&&) noexcept = default;
    
    CachedValue& operator= (const CachedValue& other) noexcept = default;
    CachedValue& operator= (CachedValue&& other) noexcept = default;


    explicit CachedValue (const allocator_type& alloc) noexcept
    {    
    }

    CachedValue (const FixedValue& v, const std::uint8_t type, const allocator_type& alloc) noexcept : value(v), valueType(FIXED)
    {
    }

    CachedValue (const VectorValue& v, const std::uint8_t type, const allocator_type& alloc) noexcept : value(v), valueType(VEC)
    {
    }


    CachedValue (const CachedValue& other, const allocator_type& alloc) noexcept : value(other.value)
    {    
    }

    CachedValue (CachedValue&& other, const allocator_type& alloc) noexcept : value(std::move(other.value))
    {    
    }

    std::variant<FixedValue, VectorValue> value;
    std::uint8_t valueType;
  };

   
  template<typename ElementT>
  fc::Vector<ElementT> createFcVectorSized(const std::size_t size)
  {
    fc::Vector<ElementT> dest(VectorMemory::getPool());
    dest.resize(size);
    return dest;
  }


  template<typename ElementT>
  fc::Vector<ElementT> createFcVectorReserved(const std::size_t size)
  {
    fc::Vector<ElementT> dest(VectorMemory::getPool());
    dest.reserve(size);
    return dest;
  }
}