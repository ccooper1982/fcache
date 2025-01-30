#pragma once

#include <cstdint>
#include <variant>
#include <list>
#include <plog/Log.h>
#include <fc/FlatBuffers.hpp>
#include <fc/Common.hpp>

//
// TODO this file is becoming chunky, requires a split
// 
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


  // Add
  template<bool SortedList>
  struct Add
  {
    Add(const flexbuffers::TypedVector& items, const fc::request::Base base, const std::int64_t position)  requires (!SortedList)
      : items(items), base(base), pos(position)
    {
    }

    Add(const flexbuffers::TypedVector& items, const fc::request::Base base, const bool itemsSorted) requires (SortedList)
      : items(items), base(base), pos(0), itemsSorted(itemsSorted)
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
    void doAdd(ListT& list) requires(ListValue<ItemT>)
    {
      // NOTE: with a flexbuffers::TypedVector we can't get an iterator,
      //       only access is via overloaded operator[]

      if constexpr (SortedList)
      {
        if (itemsSorted)
        {
          doSortedAdd<ItemT>(list);
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
        const auto size = std::ssize(list);
        const auto shift = std::min<>(pos, size);
        typename ListT::iterator it;
        
        if (base == Base_Head)
          it = std::next(list.begin(), shift);
        else          
          it = std::next(list.rbegin(), shift).base();

        /* NOTE base(), confusingly, this works because:
            - reverse iterators are implemented in terms of forward iterators (end()/begin())
            - a reverse iterator: rbegin()+i == end()-i
            - rbegin() initially is: end()-1
            - base() returns the underyling iterator (end()-i)
            - above when shift is 0, rbegin().base() return end()
            - allowing appending to the list even though rbegin() references the tail node
            
            https://stackoverflow.com/a/71366205
            https://stackoverflow.com/a/16609146
        */ 

        for (std::size_t i = 0 ; i < items.size() ; ++i)
        {
          it = list.emplace(it, items[i].As<ItemT>()); 
          ++it;
        }
      }
    }


    template<typename ItemT,typename ListT>
    void doSortedAdd(ListT& list) requires(SortedList)
    {
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
    const fc::request::Base base;
    const std::int64_t pos;
    const bool itemsSorted{false};
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

    // TODO list traits here so we can do ListTraits<IntList>::value_type
    void operator()(IntList& list)
    {
      if constexpr (std::is_same_v<value_type, fcint>)
        doRemove(list);
    }

    void operator()(UIntList& list)
    {
      if constexpr (std::is_same_v<value_type, fcuint>)
        doRemove(list);
    }

    void operator()(FloatList& list)
    {
      if constexpr (std::is_same_v<value_type, fcfloat>)
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
      using iterator_t = typename ListT::iterator;

      if (const auto [valid, itStart, itEnd] = positionsToIterators<iterator_t>(start, end, hasStop, list); valid)
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


  template<typename TargetListT, typename SourceListT>
  void doIntersectToList( TargetListT& newList, 
                          const typename SourceListT::const_iterator l1Begin, const typename SourceListT::const_iterator l1End,
                          const typename SourceListT::const_iterator l2Begin, const typename SourceListT::const_iterator l2End)
  {
    std::set_intersection(l1Begin, l1End,
                          l2Begin, l2End,
                          std::back_inserter(newList));
  }


  // intersect to a new list
  template<typename TargetListT, typename SourceListT>
  void intersect( TargetListT& newList,
                  const SourceListT& l1, const SourceListT& l2,
                  const fc::request::Range& l1Range, const fc::request::Range& l2Range) requires (std::is_same_v<std::remove_cvref_t<SourceListT>, std::remove_cvref_t<TargetListT>>)
  {
    using iterator_t = SourceListT::const_iterator;

    const auto [l1Valid, l1Begin, l1Last] = positionsToIterators<iterator_t>(l1Range.start(), l1Range.stop(), l1Range.has_stop(), l1);
    const auto [l2Valid, l2Begin, l2Last] = positionsToIterators<iterator_t>(l2Range.start(), l2Range.stop(), l2Range.has_stop(), l2);
    
    if (l1Valid && l2Valid)
    {
      doIntersectToList<TargetListT, SourceListT>(newList, l1Begin, l1Last, l2Begin, l2Last);
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

  private:
    List m_list;
    FlexType m_flexType;    
    const bool m_sorted;
  };
}
