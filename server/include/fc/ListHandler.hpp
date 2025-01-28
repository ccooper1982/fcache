#pragma once

#include <fc/FlatBuffers.hpp>
#include <fc/List.hpp>
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

  private:
    void createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType) noexcept;


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


  private:
    std::unordered_map<std::string, std::unique_ptr<FcList>> m_lists;
  };

}