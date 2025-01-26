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


  private:
    void createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType) noexcept;
    
    std::optional<Iterator> getList (const std::string& name)
    {
      if (auto it = m_lists.find(name) ; it != m_lists.end())
        return it;
      else
        return {};
    }

  private:
    std::unordered_map<std::string, std::unique_ptr<FcList>> m_lists;
  };

}