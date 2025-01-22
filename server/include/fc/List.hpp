#pragma once

#include <cstdint>
#include <variant>
#include <list>
#include <plog/Log.h>
#include <fc/FlatBuffers.hpp>


namespace fc
{ 
  using IntList = std::list<std::int64_t>;
  using UIntList = std::list<std::uint64_t>;
  using List = std::variant<IntList, UIntList>;


  template<typename V>
  concept ListValue = std::disjunction_v<std::is_same<V, std::int64_t>, std::is_same<V, std::uint64_t>, std::false_type>;


  template<typename V>
  struct AddHead
  {
    AddHead(const V& v) requires (ListValue<V>)
      : val(v) 
    {
    }

    template<typename ListT>
    void operator()(ListT& list)  // ListT: either IntList or UIntList, etc
    {
      PLOGD << "AddHead: " << val;
      list.emplace_front(val);
    }
    
  private:
    const V& val;
  };


  template<typename V>
  struct AddTail
  {
    AddTail(const V& v) requires (ListValue<V>)
      : val(v) 
    {
    }

    template<typename ListT>
    void operator()(ListT& list)  // ListT: either IntList or UIntList, etc
    {
      PLOGD << "AddTail: " << val;
      list.emplace_back(val);
    }
    
  private:
    const V& val;
  };


  class FcList
  {
  public:
    FcList(const FlexType ft, List&& list) : m_flexType(ft), m_list(std::move(list))
    {
    }

    FlexType type() const { return m_flexType; }
    List& list() { return m_list; }
    const List& list() const { return m_list; }

  private:
    FlexType m_flexType;
    List m_list;
  };
}
