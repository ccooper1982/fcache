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
  using enum fc::request::Base;


  template<typename V>
  concept ListValue = std::disjunction_v<std::is_same<V, std::int64_t>, std::is_same<V, std::uint64_t>, std::false_type>;


  /*
  template<typename V>
  struct Add
  {
    Add(const V& v, const fc::request::Base base, const std::int32_t position) requires (ListValue<V>)
      : val(v), base(base), pos(position)
    {
    }

    template<typename ListT>
    void operator()(ListT& list)  // ListT: either IntList or UIntList, etc
    {
      PLOGD << "Add: " << val;

      const auto size = list.size();
      const auto shift = std::min<>(static_cast<decltype(size)>(pos), size); // TODO is this a bad idea?

      if (base == Base_Head)
      {
        const auto it = std::next(list.begin(), shift);
        list.insert(it, val);
      }
      else if (base == Base_Tail)
      {
        const auto it = std::next(list.rbegin(), shift);
        list.insert(it.base(), val);
      }
    }
    
  private:
    const V& val;
    fc::request::Base base;
    std::int32_t pos;
  };
  */


  
  template<typename V>
  struct Add
  {
    Add(const flexbuffers::TypedVector& items, const fc::request::Base base, const std::int32_t position) requires (ListValue<V>)
      : items(items), base(base), pos(position)
    {
    }

    template<typename ListT>
    void operator()(ListT& list)  // ListT: either IntList or UIntList, etc
    {
      const auto size = list.size();
      const auto shift = std::min<>(static_cast<decltype(size)>(pos), size); // TODO is this a bad idea?
      auto it = list.begin();

      if (base == Base_Head)
      {
        PLOGD << "Base: Head, shift: " << shift;
        std::advance(it, shift);
      }
      else if (base == Base_Tail)
      {
        PLOGD << "Base: Tail, shift: " << shift;
        it = std::next(list.rbegin(), shift).base();
      }

      for (std::size_t i = 0 ; i < items.size() ; ++i)
      {
        it = list.insert(it, items[i].As<V>()); 
        ++it;
      }
    }
    
  private:
    const flexbuffers::TypedVector& items;
    fc::request::Base base;
    std::int32_t pos;
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
