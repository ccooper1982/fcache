#pragma once

#include <fc/FlatBuffers.hpp>
#include <fc/ListOperations.hpp>
#include <plog/Log.h>
#include <map>
#include <optional>


namespace fc
{
  class ListHandler
  {
    using enum fc::response::Status;
    using enum fc::response::ResponseBody;
    using enum fc::request::Base;

    using Iterator = std::unordered_map<std::string, std::unique_ptr<FcList>>::iterator;


  public:
    ListHandler() = default;


  public:
    void handle(FlatBuilder& fbb, const fc::request::ListCreate& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::ListAdd& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::ListDelete& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::ListGetRange& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::ListRemove& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::ListRemoveIf& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::ListIntersect& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::ListSet& req) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::ListAppend& req) noexcept;


  private:
    void createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType) noexcept;
    
    
    void doAddAppend( FlatBuilder& fbb, const std::string& name, const flexbuffers::TypedVector& items,
                      const bool isAppend = true, const fc::request::Base base = Base_None, const std::int64_t pos = 0, const bool itemsSorted = false);
    
    void doIntersect(FlatBuilder& fb, FlexBuilder& flxb, const fc::request::ListIntersect& req, FcList& fcList1, FcList& fcList2);


    fc::response::Status createList(const std::string& name, const fc::common::ListType type, const bool sorted);
    

    template<typename ListT>
    std::size_t doRemoveIfEquals (const int32_t start, const int32_t stop, const bool hasStop, const auto&& val, ListT& list, const bool isSorted)
    {
      if (isSorted)
        return std::visit(makeSortedRemoveIfEquals(start, stop, std::forward<decltype(val)>(val), hasStop), list);
      else
        return std::visit(makeUnsortedRemoveIfEquals(start, stop, std::forward<decltype(val)>(val), hasStop), list);
    }

    
    std::optional<Iterator> getList (const std::string& name)
    {
      if (auto it = m_lists.find(name) ; it != m_lists.end())
        return it;
      else
        return {};
    }


    std::optional<Iterator> haveList (FlatBuilder& fb, const std::string& name, const fc::response::ResponseBody bodyType)
    {
      if (const auto opt = getList(name); opt) 
        return opt;
      else  [[unlikely]]
      {
        createEmptyBodyResponse(fb, Status_Fail, bodyType); // or Status_NotExist?
        return {};
      }
    }

    
    inline bool flexTypeToListType (const FlexType type, fc::common::ListType& listType) const
    {
      using enum fc::common::ListType;
      using enum FlexType;

      bool valid = true;
      switch (type)
      {
        case FBT_VECTOR_INT:
          listType = ListType_Int;
          break;
        case FBT_VECTOR_UINT:
          listType =  ListType_UInt;
          break;
        case FBT_VECTOR_FLOAT:
          listType =  ListType_Float;
          break;
        case FBT_VECTOR_KEY:
          listType =  ListType_String;
          break;
        default:
        {
          PLOGE << "listTypeToFlexType: invalid value";
          listType = ListType_MAX;
          valid = false;
          break;
        }
      }

      return valid;
    }


  private:
    std::unordered_map<std::string, std::unique_ptr<FcList>> m_lists;
  };

}