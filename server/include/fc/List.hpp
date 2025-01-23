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


  template<typename Iterator>
  inline void listToTypedVector(FlexBuilder& fb, Iterator begin, const Iterator end)
  {
    fb.TypedVector([&fb, begin, end]() mutable
    {
      while (begin != end)
      {
        fb.Add(*begin);
        ++begin;
      }
    });
  }


  // TODO do we actually need Base? Position is signed may be possible to use to determine base
  template<typename V>
  struct Add
  {
    Add(const flexbuffers::TypedVector& items, const fc::request::Base base, const std::int64_t position) requires (ListValue<V>)
      : items(items), base(base), pos(position)
    {
    }


    template<typename ListT>
    void operator()(ListT& list)  // ListT: either IntList or UIntList, etc
    {
      const auto size = std::ssize(list);
      const auto shift = std::min<>(pos, size);
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
    std::int64_t pos;
  };


  template<typename V>
  struct GetByCount
  {
    GetByCount(FlexBuilder& fb, const std::int64_t start, const std::uint64_t count, const fc::request::Base base)
      : fb(fb), start(start), count(count), base(base)
    {
    }


    template<typename ListT>
    void operator()(ListT& list) 
    {
      const auto size = std::ssize(list);
      count = count == 0 ? size : count;
      start = std::min<>(start, size);
      
      if (base == Base_Head)  // forward iterating (head->tail)
      {
        const auto itStart = std::next(list.cbegin(), start);
        const auto itEnd = std::next(itStart, std::min<>(std::distance(itStart, list.cend()), count));

        listToTypedVector(fb, itStart, itEnd);
      }
      else // reverse iterating (tail->head)
      {
        const auto itStart = std::next(list.crbegin(), start);
        const auto itEnd = std::next(itStart, std::min<>(std::distance(itStart, list.crend()), count));

        listToTypedVector(fb, itStart, itEnd);
      }
    }


  private:
    FlexBuilder& fb;
    std::int64_t start, count;    
    const fc::request::Base base;
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
