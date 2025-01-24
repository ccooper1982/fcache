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
  inline void listToTypedVector(FlexBuilder& flxb, Iterator begin, const Iterator end)
  {
    flxb.TypedVector([&flxb, begin, end]() mutable
    {
      while (begin != end)
      {
        flxb.Add(*begin);
        ++begin;
      }
    });
  }


  template<typename Iterator>
  inline void listToTypedVector(FlexBuilder& flxb, Iterator it, const int64_t count)
  {
    flxb.TypedVector([&flxb, it, count]() mutable
    {
      for (int64_t i = 0 ; i < count ; ++i)
      {
        flxb.Add(*it);
        ++it;
      }
    });
  }


  // TODO do we actually need Base? It may be possible to use position to determine base
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


  // Get by count, always beginning from 'start'.
  //  - Can get from start, going forwards to tail
  //  - Can get from start, going backwards to head (i.e. reverse)
  //  - If count is 0, it means get everything from start to end (which is either cend() or crend())
  // Easiest/clearest way of doing this was to use Base enum which signals forwards or backwards
  // rather than trying to use 'count'.
  struct GetByCount
  {
    GetByCount(FlexBuilder& flxb, const std::int64_t start, const std::uint64_t count, const fc::request::Base base)
      : flxb(flxb), start(start), count(count), base(base)
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

        listToTypedVector(flxb, itStart, itEnd);
      }
      else // reverse iterating (tail->head)
      {
        const auto itStart = std::next(list.crbegin(), start);
        const auto itEnd = std::next(itStart, std::min<>(std::distance(itStart, list.crend()), count));

        listToTypedVector(flxb, itStart, itEnd);
      }
    }


  private:
    FlexBuilder& flxb;
    std::int64_t start, count;
    const fc::request::Base base;
  };


  // Get from index range: [start, end).
  // - Either/both start and end can be negative
  // - If base is Base_Head use cbegin(), otherwise crbegin()
  // - start must be in bounds
  struct GetByRange
  {
    GetByRange(FlexBuilder& flxb, const int64_t start, const int64_t end, const fc::request::Base base) noexcept
      : flxb(flxb), start(start), end(end), hasStop(true), base(base)
    {
    }

    GetByRange(FlexBuilder& flxb, const int64_t start, const fc::request::Base base) noexcept
      : flxb(flxb), start(start), end(0), hasStop(false), base(base)
    {
    }

    template<typename ListT>
    bool operator()(ListT& list)
    {
      bool createdBuffer = false;
      const auto size = std::ssize(list);

      // start must be inbounds (but end will be capped to cend() or crend())
      if (std::labs(start) >= size)
        return false;

      if (!hasStop)
        end = size;
      
      const std::int64_t begin  = start < 0 ? size+start : start,
                         last   = end < 0 ? std::min<>(size, std::labs(size+end)) : std::min<>(size, end);

      if (begin < last)
      {
        createdBuffer = true;

        if (base == Base_Head)
        {
          const auto itStart = std::next(list.cbegin(), begin);
          const auto count = last-begin;

          PLOGD << "Forward for " << count << " from " << *itStart;
          listToTypedVector(flxb, itStart, count);
        }
        else
        {
          const auto itStart = std::next(list.crbegin(), begin);
          const auto count = last-begin;
        
          PLOGD << "Reverse for " << count << " from " << *itStart;
          listToTypedVector(flxb, itStart, count);
        }
      }

      return  createdBuffer;
    }

  private:
    FlexBuilder& flxb;
    std::int64_t start;
    std::int64_t end;
    const bool hasStop;
    const fc::request::Base base;
  };

  
  class FcList
  {
  public:
    FcList(const FlexType ft, List&& list) noexcept : m_flexType(ft), m_list(std::move(list))
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
