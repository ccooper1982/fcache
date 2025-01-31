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

  private:
    void createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType) noexcept;
    fc::response::Status createList(const std::string& name, const fc::common::ListType type, const bool sorted);


    template<typename Condition, typename ListT>
    void doRemoveIf (const int32_t start, const int32_t stop, const bool hasStop, const typename Condition::value_type& val, ListT& list)
    {
      if (hasStop)
        std::visit(RemoveIf{start, stop, Condition{val}}, list);
      else
        std::visit(RemoveIf{start, Condition{val}}, list);
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
        createEmptyBodyResponse(fb, Status_Fail, bodyType);
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