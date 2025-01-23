// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_LISTREQUEST_FC_REQUEST_H_
#define FLATBUFFERS_GENERATED_LISTREQUEST_FC_REQUEST_H_

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/flexbuffers.h"
#include "flatbuffers/flex_flat_util.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 12 &&
              FLATBUFFERS_VERSION_REVISION == 23,
             "Non-compatible flatbuffers version included");

#include "common_generated.h"

namespace fc {
namespace request {

struct Range;
struct RangeBuilder;

struct ListCreate;
struct ListCreateBuilder;

struct ListDelete;
struct ListDeleteBuilder;

struct ListAdd;
struct ListAddBuilder;

struct ListGetN;
struct ListGetNBuilder;

enum Base : uint8_t {
  Base_Head = 0,
  Base_Tail = 1,
  Base_MIN = Base_Head,
  Base_MAX = Base_Tail
};

inline const Base (&EnumValuesBase())[2] {
  static const Base values[] = {
    Base_Head,
    Base_Tail
  };
  return values;
}

inline const char * const *EnumNamesBase() {
  static const char * const names[3] = {
    "Head",
    "Tail",
    nullptr
  };
  return names;
}

inline const char *EnumNameBase(Base e) {
  if (::flatbuffers::IsOutRange(e, Base_Head, Base_Tail)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesBase()[index];
}

struct Range FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RangeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_START = 4,
    VT_STOP = 6
  };
  int32_t start() const {
    return GetField<int32_t>(VT_START, 0);
  }
  int32_t stop() const {
    return GetField<int32_t>(VT_STOP, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int32_t>(verifier, VT_START, 4) &&
           VerifyField<int32_t>(verifier, VT_STOP, 4) &&
           verifier.EndTable();
  }
};

struct RangeBuilder {
  typedef Range Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_start(int32_t start) {
    fbb_.AddElement<int32_t>(Range::VT_START, start, 0);
  }
  void add_stop(int32_t stop) {
    fbb_.AddElement<int32_t>(Range::VT_STOP, stop, 0);
  }
  explicit RangeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Range> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Range>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Range> CreateRange(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int32_t start = 0,
    int32_t stop = 0) {
  RangeBuilder builder_(_fbb);
  builder_.add_stop(stop);
  builder_.add_start(start);
  return builder_.Finish();
}

struct ListCreate FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ListCreateBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_TYPE = 6
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  fc::common::ListType type() const {
    return static_cast<fc::common::ListType>(GetField<int8_t>(VT_TYPE, 0));
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<int8_t>(verifier, VT_TYPE, 1) &&
           verifier.EndTable();
  }
};

struct ListCreateBuilder {
  typedef ListCreate Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(ListCreate::VT_NAME, name);
  }
  void add_type(fc::common::ListType type) {
    fbb_.AddElement<int8_t>(ListCreate::VT_TYPE, static_cast<int8_t>(type), 0);
  }
  explicit ListCreateBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ListCreate> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ListCreate>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ListCreate> CreateListCreate(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    fc::common::ListType type = fc::common::ListType_Int) {
  ListCreateBuilder builder_(_fbb);
  builder_.add_name(name);
  builder_.add_type(type);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ListCreate> CreateListCreateDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    fc::common::ListType type = fc::common::ListType_Int) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return fc::request::CreateListCreate(
      _fbb,
      name__,
      type);
}

struct ListDelete FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ListDeleteBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4
  };
  const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *name() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *>(VT_NAME);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyVector(name()) &&
           verifier.VerifyVectorOfStrings(name()) &&
           verifier.EndTable();
  }
};

struct ListDeleteBuilder {
  typedef ListDelete Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> name) {
    fbb_.AddOffset(ListDelete::VT_NAME, name);
  }
  explicit ListDeleteBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ListDelete> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ListDelete>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ListDelete> CreateListDelete(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> name = 0) {
  ListDeleteBuilder builder_(_fbb);
  builder_.add_name(name);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ListDelete> CreateListDeleteDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<::flatbuffers::Offset<::flatbuffers::String>> *name = nullptr) {
  auto name__ = name ? _fbb.CreateVector<::flatbuffers::Offset<::flatbuffers::String>>(*name) : 0;
  return fc::request::CreateListDelete(
      _fbb,
      name__);
}

struct ListAdd FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ListAddBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_ITEMS = 6,
    VT_POSITION = 8,
    VT_BASE = 10
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  const ::flatbuffers::Vector<uint8_t> *items() const {
    return GetPointer<const ::flatbuffers::Vector<uint8_t> *>(VT_ITEMS);
  }
  flexbuffers::Reference items_flexbuffer_root() const {
    const auto _f = items();
    return _f ? flexbuffers::GetRoot(_f->Data(), _f->size())
              : flexbuffers::Reference();
  }
  int32_t position() const {
    return GetField<int32_t>(VT_POSITION, 0);
  }
  fc::request::Base base() const {
    return static_cast<fc::request::Base>(GetField<uint8_t>(VT_BASE, 0));
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_ITEMS) &&
           verifier.VerifyVector(items()) &&
           flexbuffers::VerifyNestedFlexBuffer(items(), verifier) &&
           VerifyField<int32_t>(verifier, VT_POSITION, 4) &&
           VerifyField<uint8_t>(verifier, VT_BASE, 1) &&
           verifier.EndTable();
  }
};

struct ListAddBuilder {
  typedef ListAdd Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(ListAdd::VT_NAME, name);
  }
  void add_items(::flatbuffers::Offset<::flatbuffers::Vector<uint8_t>> items) {
    fbb_.AddOffset(ListAdd::VT_ITEMS, items);
  }
  void add_position(int32_t position) {
    fbb_.AddElement<int32_t>(ListAdd::VT_POSITION, position, 0);
  }
  void add_base(fc::request::Base base) {
    fbb_.AddElement<uint8_t>(ListAdd::VT_BASE, static_cast<uint8_t>(base), 0);
  }
  explicit ListAddBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ListAdd> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ListAdd>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ListAdd> CreateListAdd(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<uint8_t>> items = 0,
    int32_t position = 0,
    fc::request::Base base = fc::request::Base_Head) {
  ListAddBuilder builder_(_fbb);
  builder_.add_position(position);
  builder_.add_items(items);
  builder_.add_name(name);
  builder_.add_base(base);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ListAdd> CreateListAddDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const std::vector<uint8_t> *items = nullptr,
    int32_t position = 0,
    fc::request::Base base = fc::request::Base_Head) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto items__ = items ? _fbb.CreateVector<uint8_t>(*items) : 0;
  return fc::request::CreateListAdd(
      _fbb,
      name__,
      items__,
      position,
      base);
}

struct ListGetN FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ListGetNBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_START = 6,
    VT_COUNT = 8
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  int32_t start() const {
    return GetField<int32_t>(VT_START, 0);
  }
  int32_t count() const {
    return GetField<int32_t>(VT_COUNT, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<int32_t>(verifier, VT_START, 4) &&
           VerifyField<int32_t>(verifier, VT_COUNT, 4) &&
           verifier.EndTable();
  }
};

struct ListGetNBuilder {
  typedef ListGetN Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(ListGetN::VT_NAME, name);
  }
  void add_start(int32_t start) {
    fbb_.AddElement<int32_t>(ListGetN::VT_START, start, 0);
  }
  void add_count(int32_t count) {
    fbb_.AddElement<int32_t>(ListGetN::VT_COUNT, count, 0);
  }
  explicit ListGetNBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ListGetN> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ListGetN>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ListGetN> CreateListGetN(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    int32_t start = 0,
    int32_t count = 0) {
  ListGetNBuilder builder_(_fbb);
  builder_.add_count(count);
  builder_.add_start(start);
  builder_.add_name(name);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ListGetN> CreateListGetNDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    int32_t start = 0,
    int32_t count = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return fc::request::CreateListGetN(
      _fbb,
      name__,
      start,
      count);
}

}  // namespace request
}  // namespace fc

#endif  // FLATBUFFERS_GENERATED_LISTREQUEST_FC_REQUEST_H_
