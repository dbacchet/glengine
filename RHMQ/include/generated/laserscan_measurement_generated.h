// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_LASERSCANMEASUREMENT_RHMSG_H_
#define FLATBUFFERS_GENERATED_LASERSCANMEASUREMENT_RHMSG_H_

#include "flatbuffers/flatbuffers.h"

#include "header_generated.h"

namespace RHMsg {

struct LaserscanMeasurement;

struct LaserscanMeasurement FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_HEADER = 4,
    VT_RANGES = 6
  };
  const HeaderData *header() const {
    return GetPointer<const HeaderData *>(VT_HEADER);
  }
  const flatbuffers::Vector<float> *ranges() const {
    return GetPointer<const flatbuffers::Vector<float> *>(VT_RANGES);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_HEADER) &&
           verifier.VerifyTable(header()) &&
           VerifyOffset(verifier, VT_RANGES) &&
           verifier.Verify(ranges()) &&
           verifier.EndTable();
  }
};

struct LaserscanMeasurementBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_header(flatbuffers::Offset<HeaderData> header) {
    fbb_.AddOffset(LaserscanMeasurement::VT_HEADER, header);
  }
  void add_ranges(flatbuffers::Offset<flatbuffers::Vector<float>> ranges) {
    fbb_.AddOffset(LaserscanMeasurement::VT_RANGES, ranges);
  }
  LaserscanMeasurementBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  LaserscanMeasurementBuilder &operator=(const LaserscanMeasurementBuilder &);
  flatbuffers::Offset<LaserscanMeasurement> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<LaserscanMeasurement>(end);
    return o;
  }
};

inline flatbuffers::Offset<LaserscanMeasurement> CreateLaserscanMeasurement(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<HeaderData> header = 0,
    flatbuffers::Offset<flatbuffers::Vector<float>> ranges = 0) {
  LaserscanMeasurementBuilder builder_(_fbb);
  builder_.add_ranges(ranges);
  builder_.add_header(header);
  return builder_.Finish();
}

inline flatbuffers::Offset<LaserscanMeasurement> CreateLaserscanMeasurementDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<HeaderData> header = 0,
    const std::vector<float> *ranges = nullptr) {
  return RHMsg::CreateLaserscanMeasurement(
      _fbb,
      header,
      ranges ? _fbb.CreateVector<float>(*ranges) : 0);
}

inline const RHMsg::LaserscanMeasurement *GetLaserscanMeasurement(const void *buf) {
  return flatbuffers::GetRoot<RHMsg::LaserscanMeasurement>(buf);
}

inline bool VerifyLaserscanMeasurementBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<RHMsg::LaserscanMeasurement>(nullptr);
}

inline void FinishLaserscanMeasurementBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<RHMsg::LaserscanMeasurement> root) {
  fbb.Finish(root);
}

}  // namespace RHMsg

#endif  // FLATBUFFERS_GENERATED_LASERSCANMEASUREMENT_RHMSG_H_