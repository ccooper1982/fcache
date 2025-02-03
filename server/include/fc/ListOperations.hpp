#pragma once

#include <fc/FlatBuffers.hpp>
#include <fc/Common.hpp>
#include <fc/List.hpp>


namespace fc
{
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


  template<typename It, typename ListT>
  std::tuple<bool, It, It> positionsToIterators(const int64_t start, const int64_t end, const bool hasStop, ListT& list)
  {
    using ListIt = typename ListT::iterator;
    using ListConstIt = typename ListT::const_iterator;

    constexpr bool IsConst = std::is_same_v<It, ListConstIt>;
    using Iterator = std::conditional_t<IsConst, ListConstIt, ListIt>;


    const auto [valid, begin, last] = positionsToIndices(start, end, hasStop, list);
    
    if (!valid || begin >= last)
      return {false, Iterator{}, Iterator{}};
    else
    {
      Iterator itBegin;

      if constexpr (IsConst)  
        itBegin = list.cbegin();
      else
        itBegin = list.begin();

      return {true, std::next(itBegin, begin),
                    std::next(itBegin, last)};
    }
  }


  template<typename It, typename ListT>
  It positionToStartIterator(const int64_t start, ListT& list)
  {
    // positionsToIterators() won't let start be out of bounds, but this function does,
    // returning list.end()
    const auto [valid, s, e] = positionsToIterators<It>(start, 0, false, list);
    return valid ? s : std::end(list); // assumes not a const_iterator
  }



  // Add
  template<bool SortedList>
  struct Add
  {
    Add(const flexbuffers::TypedVector& items, const fc::request::Base base, const std::int64_t position)  requires (!SortedList)
      : items(items), base(base), pos(position)
    {
    }

    Add(const flexbuffers::TypedVector& items, const fc::request::Base base, const bool itemsSorted) requires (SortedList)
      : items(items), base(base), itemsSorted(itemsSorted)
    {
    }

    Add(const flexbuffers::TypedVector& items) requires(!SortedList)
      : items(items), append(true)
    {

    }

    template<typename ListT>
    typename ListT::size_type operator()(ListT& list)
    {
      using value_type = typename ListTraits<ListT>::value_type;

      if (append)
      {
        for (std::size_t i = 0 ; i < items.size() ; ++i)
          list.emplace_back(items[i].As<value_type>()); 
      }
      else
        doAdd<value_type>(list);
    
      return list.size();
    }

  private:

    template<typename ItemT, typename ListT>
    void doAdd(ListT& list) requires(ListValue<ItemT>)
    {
      // NOTE: with a flexbuffers::TypedVector we can't get an iterator,
      //       only access is via overloaded operator[]

      if constexpr (SortedList)
      {
        if (itemsSorted)
        {
          doSourceListSortedAdd<ItemT>(list);
        }
        else
        {
          // no shortcuts
          for (std::size_t i = 0 ; i < items.size() ; ++i)
          {           
            auto val = items[i].As<ItemT>();
            const auto itPos = std::lower_bound(list.begin(), list.end(), val);
            list.emplace(itPos, std::move(val));
          }
        }
      }
      else
      {
        using iterator_t = typename ListT::iterator;

        auto it = positionToStartIterator<iterator_t>(pos, list);

        for (std::size_t i = 0 ; i < items.size() ; ++i)
        {
          it = list.emplace(it, items[i].As<ItemT>()); 
          ++it;
        }
      }
    }


    // called when items are already sorted
    template<typename ItemT,typename ListT>
    void doSourceListSortedAdd(ListT& list) requires(SortedList)
    {
      // TODO do some quickbenching to compare approaches, i.e. removing first two checks
      const auto size = list.size();

      if (const auto& highestItem = items[items.size()-1].As<ItemT>(); size && highestItem <= list.front())
      {
        for (std::size_t i = items.size() ; i ; --i)
          list.emplace_front(items[i-1].As<ItemT>());
      }
      else if (const auto& lowestItem = items[0].As<ItemT>(); size && lowestItem >= list.back())
      {
        for (std::size_t i = 0 ; i < items.size() ; ++i)
          list.emplace_back(items[i].As<ItemT>());
      }
      else
      {
        typename ListT::iterator it = list.begin();

        for (std::size_t i = 0 ; i < items.size() ; ++i)
        {
          auto val = items[i].As<ItemT>();
          const auto itPos = std::lower_bound(it, list.end(), val);
          it = list.emplace(itPos, std::move(val));
        }
      }
    }
    

  private:
    const flexbuffers::TypedVector& items;
    const fc::request::Base base{Base_None};
    const std::int64_t pos{0};
    const bool itemsSorted{false};
    const bool append{false};
  };


  // Set
  struct Set
  {
    using enum fc::response::Status;

    Set(const flexbuffers::TypedVector& items, const fc::request::Base base, const std::int64_t position)
      : items(items), base(base), pos(position)
    {
    }

    
    template<typename ListT>
    void operator()(ListT& list)
    {
      using value_t = ListTraits<ListT>::value_type;
      using iterator_t = typename ListT::iterator;
      
      if (pos < 0)
        pos = std::size(list) + pos;

      const auto [valid, begin, end] = positionsToIterators<iterator_t>(pos, pos+items.size(), true, list);
      if (!valid)
      {
        PLOGW_IF(!valid) << "List::Set is not valid range";
      }
      else
      {
        std::size_t i = 0;
        for (auto it = begin ; it != end ; ++i)
          *it++ = items[i].As<value_t>();
      }
    }
    

  private:
    const flexbuffers::TypedVector& items;
    const fc::request::Base base;
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
      : flxb(flxb), start(start), hasStop(false), base(base)
    {
    }

    template<typename ListT>
    bool operator()(ListT& list)
    {
      const auto [valid, begin, last] = positionsToIndices(start, end, hasStop, list);

      if (valid)
      {
        const auto count = last-begin;
        if (base == Base_Head)
        {
          const auto itStart = std::next(list.cbegin(), begin);
          listToTypedVector(flxb, itStart, count);
        }
        else
        {
          const auto itStart = std::next(list.crbegin(), begin);
          listToTypedVector(flxb, itStart, count);
        }
      }

      return valid;
    }

  private:
    FlexBuilder& flxb;
    const std::int64_t start;
    const std::int64_t end{0};
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
        if (last-begin == std::ssize(list))
          list.clear();
        else
        {
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
    
    explicit IsEqual (const T& v) : val(v) {}
    bool operator()(const T& a) { return a == val; }
    const T& value() const { return val; }

    const T& val; 
  };
  

  template<bool SortedList, typename Condition>
  struct RemoveIf
  {
    RemoveIf(const int64_t start, const int64_t end, Condition c) noexcept
      : start(start), end(end), hasStop(true), condition(std::move(c))
    {
    }

    RemoveIf(const int64_t start, Condition c) noexcept
      : start(start), hasStop(false), condition(std::move(c))
    {
    }
    
    template<typename ListT>
    typename ListT::size_type operator()(ListT& list)
    {
      using value_type = typename Condition::value_type;

      // need this because: RemoveIf has Condition template arg, which is templated by its value_type, 
      // so need restrict so that doRemove() is called only when ListT is for the same type as the Condition -
      // i.e. avoid executing a Condition<int> on a StringList.
      if constexpr (std::is_same_v<value_type, typename ListTraits<ListT>::value_type>)
        doRemove(list);        
      
      return std::size(list);
    }


  private:
  
    template<typename ListT>
    constexpr void doRemove (ListT& list) 
    {
      using iterator_t = typename ListT::iterator;

      const auto [valid, itBegin, itEnd] = positionsToIterators<iterator_t>(start, end, hasStop, list);
      
      if (valid)
      {
        if constexpr (SortedList)
        {
          // [itBegin,itEnd) represent what the user requested, but with a 
          // sorted list we can restrict this further
          if (const auto [itLow, itHigh] = std::equal_range(itBegin, itEnd, condition.value()) ; itLow != std::end(list))
            list.erase(itLow, itHigh);
        }
        else
        {
          const auto newEnd = std::remove_if(itBegin, itEnd, condition);
          list.erase(newEnd, itEnd);
        }
      }
    }
    
  private:
    const std::int64_t start;
    const std::int64_t end{0};
    const bool hasStop;
    Condition condition;
  };
  

  // Intersect
  template<typename ListT>
  void doIntersectToFlexBuffer( FlexBuilder& flxb, 
                  const typename ListT::const_iterator l1Begin, const typename ListT::const_iterator l1End,
                  const typename ListT::const_iterator l2Begin, const typename ListT::const_iterator l2End)
  {
    using value_t = typename ListTraits<ListT>::value_type;

    // TODO std::vector<value_t> with a sensible reserve()?
    //      I don't think we can insert directly to the flexbuffer
    std::list<value_t> result;  

    std::set_intersection(l1Begin, l1End,
                          l2Begin, l2End,
                          std::back_inserter(result));

    
    if (result.empty())    [[unlikely]]
      flxb.TypedVector([]{}); // create empty vector for clients
    else
    {
      flxb.TypedVector([&flxb, &result]
      {
        for (const auto& v : result)
          flxb.Add(v);
      });
    }

    flxb.Finish();
  }


  template<typename ListT>
  void doIntersectToList( ListT& newList, 
                          const typename ListT::const_iterator l1Begin, const typename ListT::const_iterator l1End,
                          const typename ListT::const_iterator l2Begin, const typename ListT::const_iterator l2End)
  {
    std::set_intersection(l1Begin, l1End,
                          l2Begin, l2End,
                          std::back_inserter(newList));
  }


  // intersect to a new list
  template<typename ListT>
  void intersect( ListT& newList,
                  const ListT& l1, const ListT& l2,
                  const fc::request::Range& l1Range, const fc::request::Range& l2Range)
  {
    using iterator_t = ListT::const_iterator;

    const auto [l1Valid, l1Begin, l1Last] = positionsToIterators<iterator_t>(l1Range.start(), l1Range.stop(), l1Range.has_stop(), l1);
    const auto [l2Valid, l2Begin, l2Last] = positionsToIterators<iterator_t>(l2Range.start(), l2Range.stop(), l2Range.has_stop(), l2);
    
    if (l1Valid && l2Valid)
    {
      doIntersectToList<std::remove_cvref_t<ListT>>(newList, l1Begin, l1Last, l2Begin, l2Last);
    }
  }


  // intersect to response buffer
  template<typename ListT>
  void intersect( FlexBuilder& flxb,
                  const ListT& l1, const ListT& l2,
                  const fc::request::Range& l1Range, const fc::request::Range& l2Range)
  {
    using iterator_t = typename ListT::const_iterator;

    const auto [l1Valid, l1Begin, l1Last] = positionsToIterators<iterator_t>(l1Range.start(), l1Range.stop(), l1Range.has_stop(), l1);
    const auto [l2Valid, l2Begin, l2Last] = positionsToIterators<iterator_t>(l2Range.start(), l2Range.stop(), l2Range.has_stop(), l2);
    
    if (l1Valid && l2Valid)
    {
      doIntersectToFlexBuffer<ListT>(flxb, l1Begin, l1Last, l2Begin, l2Last);
    }
  }



  // helpers
  
  // Add
  inline Add<false> makeUnsortedAdd(const flexbuffers::TypedVector& items, const fc::request::Base base, const std::int64_t position)
  {
    return Add<false>{items, base, position};
  }

  inline Add<false> makeUnsortedAppend(const flexbuffers::TypedVector& items)
  {
    return Add<false>{items};
  }

  inline Add<true> makeSortedAdd(const flexbuffers::TypedVector& items, const fc::request::Base base, const bool itemSorted)
  {
    return Add<true>{items, base, itemSorted};
  }

  // Set
  inline Set makeSet(const flexbuffers::TypedVector& items, const fc::request::Base base, const std::int64_t position)
  {
    return Set{items, base, position};
  }

  // Get
  inline GetByRange makeGetFullRange (FlexBuilder& flxb, const int64_t start, const int64_t end, const fc::request::Base base)
  {
    return GetByRange{flxb, start, end, base};
  }

  inline GetByRange makeGetPartialRange (FlexBuilder& flxb, const int64_t start, const fc::request::Base base)
  {
    return GetByRange{flxb, start, base};
  }


  // Remove  
  inline Remove makeRemoveFullRange(const int64_t start, const int64_t end)
  {
    return Remove{start, end};
  }

  inline Remove makeRemovePartialRange(const int64_t start)
  {
    return Remove{start};
  }


  // RemoveIf
  template<typename ValueT>
  inline RemoveIf<true, IsEqual<ValueT>> makeSortedRemoveIfEquals (const int64_t start, const int64_t end, const ValueT& val, const bool hasEnd)
  {
    if (hasEnd)
      return RemoveIf<true, IsEqual<ValueT>>{start, end, IsEqual{val}};
    else
      return RemoveIf<true, IsEqual<ValueT>>{start, IsEqual{val}};
  }

  template<typename ValueT>
  inline RemoveIf<false, IsEqual<ValueT>> makeUnsortedRemoveIfEquals (const int64_t start, const int64_t end, const ValueT& val, const bool hasEnd)
  {
    if (hasEnd)
      return RemoveIf<false, IsEqual<ValueT>>{start, end, IsEqual{val}};
    else
      return RemoveIf<false, IsEqual<ValueT>>{start, IsEqual{val}};
  }
}
