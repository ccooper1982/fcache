#pragma once

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/flexbuffers.h>
#include <fc/fbs/kv_response_generated.h>
#include <fc/fbs/kv_request_generated.h>
#include <fc/fbs/common_generated.h>


namespace fc
{
  using FlexBuilder = flexbuffers::Builder;
  using FlatBuilder = flatbuffers::FlatBufferBuilder;
  using BufferVector = flatbuffers::Vector<uint8_t>;
}