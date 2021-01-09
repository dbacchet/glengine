// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PROXYTOCONTROLREQUEST_RHMSG_H_
#define FLATBUFFERS_GENERATED_PROXYTOCONTROLREQUEST_RHMSG_H_

#include "flatbuffers/flatbuffers.h"

namespace RHMsg {

struct ProxyToControlRequest;

enum ProxyToControlRequestType {
  ProxyToControlRequestType_ADVANCE = 0,
  ProxyToControlRequestType_UNUPDATED_NPC_STATUS = 1,
  ProxyToControlRequestType_MIN = ProxyToControlRequestType_ADVANCE,
  ProxyToControlRequestType_MAX = ProxyToControlRequestType_UNUPDATED_NPC_STATUS
};

inline ProxyToControlRequestType (&EnumValuesProxyToControlRequestType())[2] {
  static ProxyToControlRequestType values[] = {
    ProxyToControlRequestType_ADVANCE,
    ProxyToControlRequestType_UNUPDATED_NPC_STATUS
  };
  return values;
}

inline const char **EnumNamesProxyToControlRequestType() {
  static const char *names[] = {
    "ADVANCE",
    "UNUPDATED_NPC_STATUS",
    nullptr
  };
  return names;
}

inline const char *EnumNameProxyToControlRequestType(ProxyToControlRequestType e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesProxyToControlRequestType()[index];
}

struct ProxyToControlRequest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_REQUEST = 4
  };
  ProxyToControlRequestType request() const {
    return static_cast<ProxyToControlRequestType>(GetField<int8_t>(VT_REQUEST, 0));
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_REQUEST) &&
           verifier.EndTable();
  }
};

struct ProxyToControlRequestBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_request(ProxyToControlRequestType request) {
    fbb_.AddElement<int8_t>(ProxyToControlRequest::VT_REQUEST, static_cast<int8_t>(request), 0);
  }
  ProxyToControlRequestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ProxyToControlRequestBuilder &operator=(const ProxyToControlRequestBuilder &);
  flatbuffers::Offset<ProxyToControlRequest> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ProxyToControlRequest>(end);
    return o;
  }
};

inline flatbuffers::Offset<ProxyToControlRequest> CreateProxyToControlRequest(
    flatbuffers::FlatBufferBuilder &_fbb,
    ProxyToControlRequestType request = ProxyToControlRequestType_ADVANCE) {
  ProxyToControlRequestBuilder builder_(_fbb);
  builder_.add_request(request);
  return builder_.Finish();
}

inline const RHMsg::ProxyToControlRequest *GetProxyToControlRequest(const void *buf) {
  return flatbuffers::GetRoot<RHMsg::ProxyToControlRequest>(buf);
}

inline bool VerifyProxyToControlRequestBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<RHMsg::ProxyToControlRequest>(nullptr);
}

inline void FinishProxyToControlRequestBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<RHMsg::ProxyToControlRequest> root) {
  fbb.Finish(root);
}

}  // namespace RHMsg

#endif  // FLATBUFFERS_GENERATED_PROXYTOCONTROLREQUEST_RHMSG_H_