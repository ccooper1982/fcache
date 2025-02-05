#pragma once

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/flexbuffers.h>
#include <fc/fbs/response_generated.h>
#include <fc/fbs/request_generated.h>
#include <fc/fbs/common_generated.h>
#include <fc/fbs/list_request_generated.h>
#include <fc/fbs/list_response_generated.h>
#include <plog/Log.h>


namespace fc
{
  using FlexBuilder = flexbuffers::Builder;
  using FlatBuilder = flatbuffers::FlatBufferBuilder;
  using BufferVector = flatbuffers::Vector<uint8_t>;
  using FlexType = flexbuffers::Type;


  inline void createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType) noexcept
  {
    try
    {
      fbb.Finish(fc::response::CreateResponse (fbb, status, bodyType));
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
    }
  }
}