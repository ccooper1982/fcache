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

  
  // TODO do we actually need Base? Position is signed may be possible to use to determine base
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


  template<typename V>
  struct GetByCount
  {
    GetByCount(const std::int32_t start, const std::int32_t count, FlexBuilder& fb) : start(start), count(count), fb(fb)
    {

    }

    template<typename ListT>
    void operator()(ListT& list) 
    {
      const auto size = list.size();
      const auto shift = std::min<>(static_cast<decltype(size)>(start), size);

      typename ListT::const_iterator itStart;
      typename ListT::const_iterator itStop;

      if (start < 0)
      {
        itStart = std::next(list.crbegin(), shift).base();
      }
      else
      {
        itStart = std::next(list.cbegin(), shift);
      }

      fb.TypedVector([this, size, itNode = itStart]() mutable
      {
        const int32_t last = count == 0 ? static_cast<int32_t>(size) : count;

        for (int32_t i = 0 ; i < last ; ++i)
        {
          fb.Add(*itNode);
          ++itNode;
        } 
      });
    }


  private:
    std::int32_t start, count;
    FlexBuilder& fb;
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
