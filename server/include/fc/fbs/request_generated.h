// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_REQUEST_FC_REQUEST_H_
#define FLATBUFFERS_GENERATED_REQUEST_FC_REQUEST_H_

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
#include "kv_request_generated.h"
#include "list_request_generated.h"

namespace fc {
namespace request {

struct Request;
struct RequestBuilder;

enum RequestBody : uint8_t {
  RequestBody_NONE = 0,
  RequestBody_KVSet = 1,
  RequestBody_KVGet = 2,
  RequestBody_KVRmv = 3,
  RequestBody_KVAdd = 4,
  RequestBody_KVCount = 5,
  RequestBody_KVContains = 6,
  RequestBody_KVClear = 7,
  RequestBody_KVClearSet = 8,
  RequestBody_ListCreate = 9,
  RequestBody_ListAdd = 10,
  RequestBody_MIN = RequestBody_NONE,
  RequestBody_MAX = RequestBody_ListAdd
};

inline const RequestBody (&EnumValuesRequestBody())[11] {
  static const RequestBody values[] = {
    RequestBody_NONE,
    RequestBody_KVSet,
    RequestBody_KVGet,
    RequestBody_KVRmv,
    RequestBody_KVAdd,
    RequestBody_KVCount,
    RequestBody_KVContains,
    RequestBody_KVClear,
    RequestBody_KVClearSet,
    RequestBody_ListCreate,
    RequestBody_ListAdd
  };
  return values;
}

inline const char * const *EnumNamesRequestBody() {
  static const char * const names[12] = {
    "NONE",
    "KVSet",
    "KVGet",
    "KVRmv",
    "KVAdd",
    "KVCount",
    "KVContains",
    "KVClear",
    "KVClearSet",
    "ListCreate",
    "ListAdd",
    nullptr
  };
  return names;
}

inline const char *EnumNameRequestBody(RequestBody e) {
  if (::flatbuffers::IsOutRange(e, RequestBody_NONE, RequestBody_ListAdd)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesRequestBody()[index];
}

template<typename T> struct RequestBodyTraits {
  static const RequestBody enum_value = RequestBody_NONE;
};

template<> struct RequestBodyTraits<fc::request::KVSet> {
  static const RequestBody enum_value = RequestBody_KVSet;
};

template<> struct RequestBodyTraits<fc::request::KVGet> {
  static const RequestBody enum_value = RequestBody_KVGet;
};

template<> struct RequestBodyTraits<fc::request::KVRmv> {
  static const RequestBody enum_value = RequestBody_KVRmv;
};

template<> struct RequestBodyTraits<fc::request::KVAdd> {
  static const RequestBody enum_value = RequestBody_KVAdd;
};

template<> struct RequestBodyTraits<fc::request::KVCount> {
  static const RequestBody enum_value = RequestBody_KVCount;
};

template<> struct RequestBodyTraits<fc::request::KVContains> {
  static const RequestBody enum_value = RequestBody_KVContains;
};

template<> struct RequestBodyTraits<fc::request::KVClear> {
  static const RequestBody enum_value = RequestBody_KVClear;
};

template<> struct RequestBodyTraits<fc::request::KVClearSet> {
  static const RequestBody enum_value = RequestBody_KVClearSet;
};

template<> struct RequestBodyTraits<fc::request::ListCreate> {
  static const RequestBody enum_value = RequestBody_ListCreate;
};

template<> struct RequestBodyTraits<fc::request::ListAdd> {
  static const RequestBody enum_value = RequestBody_ListAdd;
};

bool VerifyRequestBody(::flatbuffers::Verifier &verifier, const void *obj, RequestBody type);
bool VerifyRequestBodyVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<uint8_t> *types);

struct Request FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RequestBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_IDENT = 4,
    VT_BODY_TYPE = 6,
    VT_BODY = 8
  };
  fc::common::Ident ident() const {
    return static_cast<fc::common::Ident>(GetField<int8_t>(VT_IDENT, 0));
  }
  fc::request::RequestBody body_type() const {
    return static_cast<fc::request::RequestBody>(GetField<uint8_t>(VT_BODY_TYPE, 0));
  }
  const void *body() const {
    return GetPointer<const void *>(VT_BODY);
  }
  template<typename T> const T *body_as() const;
  const fc::request::KVSet *body_as_KVSet() const {
    return body_type() == fc::request::RequestBody_KVSet ? static_cast<const fc::request::KVSet *>(body()) : nullptr;
  }
  const fc::request::KVGet *body_as_KVGet() const {
    return body_type() == fc::request::RequestBody_KVGet ? static_cast<const fc::request::KVGet *>(body()) : nullptr;
  }
  const fc::request::KVRmv *body_as_KVRmv() const {
    return body_type() == fc::request::RequestBody_KVRmv ? static_cast<const fc::request::KVRmv *>(body()) : nullptr;
  }
  const fc::request::KVAdd *body_as_KVAdd() const {
    return body_type() == fc::request::RequestBody_KVAdd ? static_cast<const fc::request::KVAdd *>(body()) : nullptr;
  }
  const fc::request::KVCount *body_as_KVCount() const {
    return body_type() == fc::request::RequestBody_KVCount ? static_cast<const fc::request::KVCount *>(body()) : nullptr;
  }
  const fc::request::KVContains *body_as_KVContains() const {
    return body_type() == fc::request::RequestBody_KVContains ? static_cast<const fc::request::KVContains *>(body()) : nullptr;
  }
  const fc::request::KVClear *body_as_KVClear() const {
    return body_type() == fc::request::RequestBody_KVClear ? static_cast<const fc::request::KVClear *>(body()) : nullptr;
  }
  const fc::request::KVClearSet *body_as_KVClearSet() const {
    return body_type() == fc::request::RequestBody_KVClearSet ? static_cast<const fc::request::KVClearSet *>(body()) : nullptr;
  }
  const fc::request::ListCreate *body_as_ListCreate() const {
    return body_type() == fc::request::RequestBody_ListCreate ? static_cast<const fc::request::ListCreate *>(body()) : nullptr;
  }
  const fc::request::ListAdd *body_as_ListAdd() const {
    return body_type() == fc::request::RequestBody_ListAdd ? static_cast<const fc::request::ListAdd *>(body()) : nullptr;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_IDENT, 1) &&
           VerifyField<uint8_t>(verifier, VT_BODY_TYPE, 1) &&
           VerifyOffset(verifier, VT_BODY) &&
           VerifyRequestBody(verifier, body(), body_type()) &&
           verifier.EndTable();
  }
};

template<> inline const fc::request::KVSet *Request::body_as<fc::request::KVSet>() const {
  return body_as_KVSet();
}

template<> inline const fc::request::KVGet *Request::body_as<fc::request::KVGet>() const {
  return body_as_KVGet();
}

template<> inline const fc::request::KVRmv *Request::body_as<fc::request::KVRmv>() const {
  return body_as_KVRmv();
}

template<> inline const fc::request::KVAdd *Request::body_as<fc::request::KVAdd>() const {
  return body_as_KVAdd();
}

template<> inline const fc::request::KVCount *Request::body_as<fc::request::KVCount>() const {
  return body_as_KVCount();
}

template<> inline const fc::request::KVContains *Request::body_as<fc::request::KVContains>() const {
  return body_as_KVContains();
}

template<> inline const fc::request::KVClear *Request::body_as<fc::request::KVClear>() const {
  return body_as_KVClear();
}

template<> inline const fc::request::KVClearSet *Request::body_as<fc::request::KVClearSet>() const {
  return body_as_KVClearSet();
}

template<> inline const fc::request::ListCreate *Request::body_as<fc::request::ListCreate>() const {
  return body_as_ListCreate();
}

template<> inline const fc::request::ListAdd *Request::body_as<fc::request::ListAdd>() const {
  return body_as_ListAdd();
}

struct RequestBuilder {
  typedef Request Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_ident(fc::common::Ident ident) {
    fbb_.AddElement<int8_t>(Request::VT_IDENT, static_cast<int8_t>(ident), 0);
  }
  void add_body_type(fc::request::RequestBody body_type) {
    fbb_.AddElement<uint8_t>(Request::VT_BODY_TYPE, static_cast<uint8_t>(body_type), 0);
  }
  void add_body(::flatbuffers::Offset<void> body) {
    fbb_.AddOffset(Request::VT_BODY, body);
  }
  explicit RequestBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Request> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Request>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Request> CreateRequest(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    fc::common::Ident ident = fc::common::Ident_KV,
    fc::request::RequestBody body_type = fc::request::RequestBody_NONE,
    ::flatbuffers::Offset<void> body = 0) {
  RequestBuilder builder_(_fbb);
  builder_.add_body(body);
  builder_.add_body_type(body_type);
  builder_.add_ident(ident);
  return builder_.Finish();
}

inline bool VerifyRequestBody(::flatbuffers::Verifier &verifier, const void *obj, RequestBody type) {
  switch (type) {
    case RequestBody_NONE: {
      return true;
    }
    case RequestBody_KVSet: {
      auto ptr = reinterpret_cast<const fc::request::KVSet *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case RequestBody_KVGet: {
      auto ptr = reinterpret_cast<const fc::request::KVGet *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case RequestBody_KVRmv: {
      auto ptr = reinterpret_cast<const fc::request::KVRmv *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case RequestBody_KVAdd: {
      auto ptr = reinterpret_cast<const fc::request::KVAdd *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case RequestBody_KVCount: {
      auto ptr = reinterpret_cast<const fc::request::KVCount *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case RequestBody_KVContains: {
      auto ptr = reinterpret_cast<const fc::request::KVContains *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case RequestBody_KVClear: {
      auto ptr = reinterpret_cast<const fc::request::KVClear *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case RequestBody_KVClearSet: {
      auto ptr = reinterpret_cast<const fc::request::KVClearSet *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case RequestBody_ListCreate: {
      auto ptr = reinterpret_cast<const fc::request::ListCreate *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case RequestBody_ListAdd: {
      auto ptr = reinterpret_cast<const fc::request::ListAdd *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyRequestBodyVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (::flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyRequestBody(
        verifier,  values->Get(i), types->GetEnum<RequestBody>(i))) {
      return false;
    }
  }
  return true;
}

inline const fc::request::Request *GetRequest(const void *buf) {
  return ::flatbuffers::GetRoot<fc::request::Request>(buf);
}

inline const fc::request::Request *GetSizePrefixedRequest(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<fc::request::Request>(buf);
}

inline bool VerifyRequestBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<fc::request::Request>(nullptr);
}

inline bool VerifySizePrefixedRequestBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<fc::request::Request>(nullptr);
}

inline void FinishRequestBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fc::request::Request> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedRequestBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<fc::request::Request> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace request
}  // namespace fc

#endif  // FLATBUFFERS_GENERATED_REQUEST_FC_REQUEST_H_
