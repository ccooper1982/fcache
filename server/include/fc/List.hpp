#pragma once

#include <cstdint>
#include <variant>
#include <list>
#include <plog/Log.h>
#include <fc/FlatBuffers.hpp>
#include <fc/Common.hpp>


namespace fc
{ 
  // TODO should probably have a traits class, to lookup type info for each list type, so we can do:
  //   ListTraits<IntList>::value_type
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


  template<typename Iterator>
  void listToTypedVector(FlexBuilder& flxb, Iterator begin, const Iterator end)
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
  void listToTypedVector(FlexBuilder& flxb, Iterator it, const int64_t count)
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


  template<typename ListT>
  std::tuple<bool, int64_t, int64_t> positionsToIndices (const int64_t start, int64_t end, const bool hasStop, ListT& list)
  {
    const auto size = std::ssize(list);

    if ((start >= 0 && start >= size) || std::labs(start) > size)
      return {false, 0, 0};

    if (!hasStop)
      end = size;
    
    const std::int64_t  begin  = start < 0 ? size+start : start,
                        last   = end < 0 ? std::min<>(size, std::labs(size+end)) : std::min<>(size, end);

    return{true, begin, last};
  }


  template<typename ListT>
  std::tuple<bool, typename ListT::iterator, typename ListT::iterator> positionsToIterators(const int64_t start, const int64_t end, const bool hasStop, ListT& list)
  {
    const auto [valid, begin, last] = positionsToIndices(start, end, hasStop, list);

    if (!valid || begin >= last)
      return {false, list.end(), list.end()};
    else
    {
      const auto itStart = std::next(list.begin(), begin);
      const auto itEnd = std::next(list.begin(), last);
      return {true, itStart, itEnd};
    }
  }


  // Add
  struct Add
  {
    Add(const flexbuffers::TypedVector& items, const fc::request::Base base, const std::int64_t position)
      : items(items), base(base), pos(position)
    {
    }

    void operator()(IntList& list)
    {
      doAdd<fcint>(list);
    }

    void operator()(UIntList& list) 
    {
      doAdd<fcuint>(list);
    }

    void operator()(FloatList& list) 
    {
      doAdd<fcfloat>(list);
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


  // Get Range
  // Range: [start, end).
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
      const auto [valid, begin, last] = positionsToIndices(start, end, hasStop, list);

      if (valid)
      {
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

      return valid;
    }

  private:
    FlexBuilder& flxb;
    std::int64_t start;
    std::int64_t end;
    const bool hasStop;
    const fc::request::Base base;
  };


  // Remove  
  struct Remove
  {
    Remove(const int64_t start, const int64_t end) noexcept
      : start(start), end(end), hasStop(true)
    {
    }

    Remove(const int64_t start) noexcept
      : start(start), end(0), hasStop(false)
    {
    }

    
    template<typename ListT>
    void operator()(ListT& list)
    {
      const auto [valid, begin, last] = positionsToIndices(start, end, hasStop, list);

      if (valid)
      {
        // if erasing all nodes
        if (begin == 0 && last == std::ssize(list))
        {
          PLOGD << "Clearing";
          list.clear();
        }
        else
        {
          PLOGD << "Removing: " << begin << " to " << last;

          const auto itStart = std::next(list.begin(), begin);
          const auto itEnd = std::next(list.begin(), last);
          list.erase(itStart, itEnd);
        }
      }
    }


  private:
    std::int64_t start;
    std::int64_t end;
    const bool hasStop;
  };
  
  
  // RemoveIf
  
  template<typename T>
  struct IsEqual
  {
    using value_type = T;

    
    explicit IsEqual (const T v) : val(std::move(v)) {}
    bool operator()(const T& a) { return a == val; }

    // TODO or const T& val? doesn't matter for primitives, want to avoid copy strings, but
    T val; 
  };
  

  template<typename Condition>
  struct RemoveIf
  {
    using value_type = typename Condition::value_type;


    RemoveIf(const int64_t start, const int64_t end, Condition c) noexcept
      : start(start), end(end), hasStop(true), condition(std::move(c))
    {
    }

    RemoveIf(const int64_t start, Condition c) noexcept
      : start(start), end(0), hasStop(false), condition(std::move(c))
    {
    }


    void operator()(IntList& list)
    {
      if constexpr (std::is_same_v<value_type, int64_t>)
        doRemove(list);
    }

    void operator()(UIntList& list)
    {
      if constexpr (std::is_same_v<value_type, uint64_t>)
        doRemove(list);
    }

    void operator()(FloatList& list)
    {
      if constexpr (std::is_same_v<value_type, float>)
        doRemove(list);
    }

    void operator()(StringList& list)
    {
      if constexpr (std::is_same_v<value_type, std::string>)
        doRemove(list);
    }


  private:
  
    template<typename ListT>
    constexpr void doRemove (ListT& list) 
    {
      if (const auto [valid, itStart, itEnd] = positionsToIterators(start, end, hasStop, list); valid)
      {
        const auto newEnd = std::remove_if(itStart, itEnd, condition);
        list.erase(newEnd, itEnd);
      } 
    }
    
  private:
    const std::int64_t start;
    std::int64_t end;
    const bool hasStop;
    Condition condition;
  };
  

  // Holds everything we need to know about a list.
  // The list is a variant so we can manage a list of different
  // types. They are interacted with via std::visit().
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
