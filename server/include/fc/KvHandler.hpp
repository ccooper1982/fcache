#pragma once

#include <fc/FlatBuffers.hpp>
#include <fc/Map.hpp>

namespace fc
{
  class KvHandler
  {
    using enum fc::response::Status;
    using enum fc::response::ResponseBody;


  public:
    KvHandler() = default;


  public:
    void handle(FlatBuilder& fbb, const fc::request::KVSet& set) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVGet& get) noexcept;
    void handle(FlatBuilder& fbb, const fc::request::KVRmv& rmv) noexcept;


  private:
    void createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType) noexcept;


  private:
    CacheMap m_map;
  };

}