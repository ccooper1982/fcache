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
  using FloatList = std::list<float>;
  using StringList = std::list<std::string>;
  using List = std::variant<IntList, UIntList, StringList, FloatList>;
  using enum fc::request::Base;


  template<typename V>
  concept ListValue = std::disjunction_v< std::is_same<V, std::int64_t>,
                                          std::is_same<V, std::uint64_t>,
                                          std::is_same<V, float>,
                                          std::is_same<V, std::string>,
                                          std::false_type>;


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
  struct Add
  {
    Add(const flexbuffers::TypedVector& items, const fc::request::Base base, const std::int64_t position) //requires (ListValue<V>)
      : items(items), base(base), pos(position)
    {
    }

    void operator()(IntList& list)
    {
      doAdd<std::int64_t>(list);
    }

    void operator()(UIntList& list) 
    {
      doAdd<std::uint64_t>(list);
    }

    void operator()(FloatList& list) 
    {
      doAdd<float>(list);
    }

    void operator()(StringList& list)
    {
      doAdd<std::string>(list);
    }

  private:
    template<typename ItemT, typename ListT>
    void doAdd(ListT& list)
    {
      const auto size = std::ssize(list);
      const auto shift = std::min<>(pos, size);
      auto it = list.begin();

      if (base == Base_Head)
        std::advance(it, shift);
      else if (base == Base_Tail)
        it = std::next(list.rbegin(), shift).base();

      for (std::size_t i = 0 ; i < items.size() ; ++i)
      {
        it = list.insert(it, items[i].As<ItemT>()); 
        ++it;
      }
    }
    
  private:
    const flexbuffers::TypedVector& items;
    fc::request::Base base;
    std::int64_t pos;
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
    FcList(IntList&& list) noexcept : m_flexType(FlexType::FBT_VECTOR_INT), m_list(std::move(list))
    {
    }
    FcList(UIntList&& list) noexcept : m_flexType(FlexType::FBT_VECTOR_UINT), m_list(std::move(list))
    {
    }
    FcList(FloatList&& list) noexcept : m_flexType(FlexType::FBT_VECTOR_FLOAT), m_list(std::move(list))
    {
    }
    FcList(StringList&& list) noexcept : m_flexType(FlexType::FBT_VECTOR_KEY), m_list(std::move(list))
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
