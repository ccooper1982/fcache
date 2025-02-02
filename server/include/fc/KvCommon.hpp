#pragma once

#include <variant>
#include <fc/FlatBuffers.hpp>
#include <fc/Memory.hpp>
#include <fc/Common.hpp>

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
    template<typename ScalarT>
    FixedValue (const ScalarT v, ExtractFixedF extract) noexcept :
      value(v),
      extract(std::move(extract))
    {
    }

    FixedValue() = default;
    FixedValue(const FixedValue&) noexcept = default;
    FixedValue(FixedValue&&) noexcept = default;

    FixedValue& operator= (const FixedValue& other) noexcept = default;
    FixedValue& operator= (FixedValue&& other) noexcept = default;


    std::variant<fcint, fcuint, fcfloat, fcbool> value;
    ExtractFixedF extract;
  };

   
  struct VectorValue
  {
    using allocator_type = std::pmr::polymorphic_allocator<uint8_t>;
    

    VectorValue() = default;
    VectorValue(const VectorValue&) noexcept = default;
    VectorValue(VectorValue&&) noexcept = default;

    VectorValue& operator= (const VectorValue& other) noexcept = default;
    VectorValue& operator= (VectorValue&& other) noexcept = default;


    explicit VectorValue(const allocator_type& alloc) : data(alloc)
    {

    }

    VectorValue(const std::pmr::vector<uint8_t>& d, const allocator_type& alloc) noexcept :
      data(d, alloc)
    {
      
    }

    VectorValue(std::pmr::vector<uint8_t>&& d, const allocator_type& alloc) noexcept :
      data(std::move(d), alloc)
    {
      
    }

    VectorValue (const VectorValue& other, const allocator_type& alloc) noexcept :
      data(other.data, alloc),
      extract(other.extract),
      type(other.type) 
    {    
    }

    VectorValue (VectorValue&& other, const allocator_type& alloc) noexcept :
      data(std::move(other.data)),
      extract(other.extract),
      type(other.type)
    {    
    }
    

    std::pmr::vector<uint8_t> data;
    ExtractVectorF extract;
    FlexType type;
  };

  
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

    CachedValue (const FixedValue& v, const allocator_type& alloc) noexcept :
      value(std::in_place_type<FixedValue>, v),
      valueType(FIXED)
    {
    }

    CachedValue (FixedValue&& v, const allocator_type& alloc) noexcept :
      value(std::in_place_type<FixedValue>, v),
      valueType(FIXED)
    {
    }

    CachedValue (const VectorValue& v, const allocator_type& alloc) noexcept :
      value(std::in_place_type<VectorValue>, v, alloc),
      valueType(VEC)
    {
    }

    CachedValue (VectorValue&& v, const allocator_type& alloc) noexcept :
      value(std::in_place_type<VectorValue>, std::move(v), alloc),
      valueType(VEC)
    {
    }

    CachedValue (const CachedValue& other, const allocator_type& alloc) noexcept : value(other.value)
    {    
    }

    CachedValue (CachedValue&& other, const allocator_type& alloc) noexcept :
      value(std::move(other.value)),
      valueType(other.valueType)
    {    
    }


    std::variant<FixedValue, VectorValue> value;
    std::uint8_t valueType;
  };
  
}