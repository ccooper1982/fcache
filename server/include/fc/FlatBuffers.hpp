#pragma once

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/flexbuffers.h>
#include "fbs/kv_response_generated.h"
#include "fbs/kv_request_generated.h"
#include "fbs/common_generated.h"


namespace fc
{
  using FlexBuilder = flexbuffers::Builder;
  using FlatBuilder = flatbuffers::FlatBufferBuilder;
}