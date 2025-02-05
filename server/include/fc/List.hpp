#pragma once

#include <cstdint>
#include <variant>
#include <list>
#include <plog/Log.h>
#include <fc/FlatBuffers.hpp>
#include <fc/Common.hpp>


namespace fc
{ 
  using IntList     = std::list<fcint>;
  using UIntList    = std::list<fcuint>;
  using FloatList   = std::list<fcfloat>;
  using StringList  = std::list<fcstring>;
  using List = std::variant<IntList, UIntList, StringList, FloatList>;
  
  template<typename V>
  concept ListValue = std::disjunction_v< std::is_same<V, fcint>,
                                          std::is_same<V, fcuint>,
                                          std::is_same<V, fcfloat>,
                                          std::is_same<V, fcstring>,
                                          std::false_type>;


  template<typename T>
  struct ListTraitsImpl
  {
    // ListT is the list, i.e. IntList, UIntList, etc
    // value_type is what the std::list<T> stores
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
    using ConstIterator = typename T::const_iterator;
  };

  template<>
  struct ListTraitsImpl<IntList> : public ListCommonTraits<IntList>
  {
    using value_type = fcint;
    static constexpr FlexType flexType() { return FlexType::FBT_VECTOR_INT; }
  };

  template<>
  struct ListTraitsImpl<UIntList> : public ListCommonTraits<UIntList>
  {
    using value_type = fcuint;
    static constexpr FlexType flexType() { return FlexType::FBT_VECTOR_UINT; }
  };

  template<>
  struct ListTraitsImpl<FloatList> : public ListCommonTraits<FloatList>
  {
    using value_type = fcfloat;
    static constexpr FlexType flexType() { return FlexType::FBT_VECTOR_FLOAT; }
  };

  template<>
  struct ListTraitsImpl<StringList> : public ListCommonTraits<StringList>
  {
    using value_type = fcstring;
    static constexpr FlexType flexType() { return FlexType::FBT_VECTOR_KEY; }
  };

  template<typename T>
  using ListTraits = ListTraitsImpl<std::decay_t<T>>;


  // Holds everything we need to know about a list.
  // User created lists can be a list of int, uint, float or string.
  // They are stored in a map<std::string, List>, where List is a variant.
  // Actions on a list is handled using std::visit(), with handlers in 
  // ListOperations.hpp.
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
    const FlexType m_flexType;    
    const bool m_sorted;
  };
}
