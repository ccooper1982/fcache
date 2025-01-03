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
    void handle(FlatBuilder& fbb, const fc::request::KVSet& set);
    void handle(FlatBuilder& fbb, const fc::request::KVGet& get);
    void handle(FlatBuilder& fbb, const fc::request::KVRmv& rmv);


  private:
    void createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType);


  private:
    CacheMap m_map;
  };

}