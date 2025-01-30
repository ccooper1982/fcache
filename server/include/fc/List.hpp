#pragma once

#include <cstdint>
#include <variant>
#include <list>
#include <plog/Log.h>
#include <fc/FlatBuffers.hpp>
#include <fc/Common.hpp>


namespace fc
{ 
  using IntList = std::list<fcint>;
  using UIntList = std::list<fcuint>;
  using FloatList = std::list<fcfloat>;
  using StringList = std::list<std::string>;
  using List = std::variant<IntList, UIntList, StringList, FloatList>;
  using enum fc::request::Base;


  template<typename V>
  concept ListValue = std::disjunction_v< std::is_same<V, fcint>,
                                          std::is_same<V, fcuint>,
                                          std::is_same<V, fcfloat>,
                                          std::is_same<V, std::string>,
                                          std::false_type>;


  template<typename T>
  struct ListTraits
  {
    // value_type is what the std::list<T> stores
    // ListT is the variant in List
    // flexType() returns the FlexBuffer type for the list in requests/responses

    static constexpr FlexType flexType()
    {
      static_assert(false, "T not supported by List variant");
      return FlexType::FBT_NULL;
    }
  };

  
  template<typename T>
  struct ListCommonTraits
  {
    using ListT = T;
    using Iterator = typename T::iterator;
  };

  template<>
  struct ListTraits<IntList> : public ListCommonTraits<IntList>
  {
    using value_type = int64_t;
    static constexpr FlexType flexType() { return FlexType::FBT_VECTOR_INT; }
  };

  template<>
  struct ListTraits<UIntList> : public ListCommonTraits<UIntList>
  {
    using value_type = uint64_t;
    static constexpr FlexType flexType() { return FlexType::FBT_VECTOR_UINT; }
  };

  template<>
  struct ListTraits<FloatList> : public ListCommonTraits<FloatList>
  {
    using value_type = float;
    static constexpr FlexType flexType() { return FlexType::FBT_VECTOR_FLOAT; }
  };

  template<>
  struct ListTraits<StringList> : public ListCommonTraits<StringList>
  {
    using value_type = std::string;
    static constexpr FlexType flexType() { return FlexType::FBT_VECTOR_KEY; }
  };


  // Holds everything we need to know about a list.
  // The list is a variant so we can manage a list of different
  // types. They are interacted with via std::visit().
  class FcList
  {
  public:
    template<typename ListT>
    FcList(ListT&& list, const bool sorted) noexcept :
      m_list(std::move(list)),
      m_flexType(ListTraits<ListT>::flexType()),
      m_sorted(sorted)
    {

    }
    
    FlexType type() const noexcept { return m_flexType; }
    List& list() noexcept { return m_list; }
    const List& list() const noexcept { return m_list; }
    bool isSorted () const noexcept { return m_sorted; }


    bool operator==(const FcList& a)
    {
      return a.type() == type() && a.isSorted() == isSorted();
    }

    bool operator!=(const FcList& a)
    {
      return !(*this == a);
    }


    bool canIntersectWith (const FcList& other)
    {
     return isSorted() && other.isSorted() && type() == other.type();
    }

  private:
    List m_list;
    FlexType m_flexType;    
    const bool m_sorted;
  };
}
