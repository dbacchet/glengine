// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_DEPTHIMAGEMEASUREMENT_RHMSG_H_
#define FLATBUFFERS_GENERATED_DEPTHIMAGEMEASUREMENT_RHMSG_H_

#include "flatbuffers/flatbuffers.h"

#include "header_generated.h"

namespace RHMsg {

struct DepthPixel;

struct DepthImageMeasurement;

MANUALLY_ALIGNED_STRUCT(4) DepthPixel FLATBUFFERS_FINAL_CLASS {
 private:
  float depth_;
  float intensity_;

 public:
  DepthPixel() {
    memset(this, 0, sizeof(DepthPixel));
  }
  DepthPixel(const DepthPixel &_o) {
    memcpy(this, &_o, sizeof(DepthPixel));
  }
  DepthPixel(float _depth, float _intensity)
      : depth_(flatbuffers::EndianScalar(_depth)),
        intensity_(flatbuffers::EndianScalar(_intensity)) {
  }
  float depth() const {
    return flatbuffers::EndianScalar(depth_);
  }
  float intensity() const {
    return flatbuffers::EndianScalar(intensity_);
  }
};
STRUCT_END(DepthPixel, 8);

struct DepthImageMeasurement FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_HEADER = 4,
    VT_PIXELS = 6,
    VT_ENCODING = 8,
    VT_WIDTH = 10,
    VT_HEIGHT = 12
  };
  const HeaderData *header() const {
    return GetPointer<const HeaderData *>(VT_HEADER);
  }
  const flatbuffers::Vector<const DepthPixel *> *pixels() const {
    return GetPointer<const flatbuffers::Vector<const DepthPixel *> *>(VT_PIXELS);
  }
  const flatbuffers::String *encoding() const {
    return GetPointer<const flatbuffers::String *>(VT_ENCODING);
  }
  int32_t width() const {
    return GetField<int32_t>(VT_WIDTH, 0);
  }
  int32_t height() const {
    return GetField<int32_t>(VT_HEIGHT, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_HEADER) &&
           verifier.VerifyTable(header()) &&
           VerifyOffset(verifier, VT_PIXELS) &&
           verifier.Verify(pixels()) &&
           VerifyOffset(verifier, VT_ENCODING) &&
           verifier.Verify(encoding()) &&
           VerifyField<int32_t>(verifier, VT_WIDTH) &&
           VerifyField<int32_t>(verifier, VT_HEIGHT) &&
           verifier.EndTable();
  }
};

struct DepthImageMeasurementBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_header(flatbuffers::Offset<HeaderData> header) {
    fbb_.AddOffset(DepthImageMeasurement::VT_HEADER, header);
  }
  void add_pixels(flatbuffers::Offset<flatbuffers::Vector<const DepthPixel *>> pixels) {
    fbb_.AddOffset(DepthImageMeasurement::VT_PIXELS, pixels);
  }
  void add_encoding(flatbuffers::Offset<flatbuffers::String> encoding) {
    fbb_.AddOffset(DepthImageMeasurement::VT_ENCODING, encoding);
  }
  void add_width(int32_t width) {
    fbb_.AddElement<int32_t>(DepthImageMeasurement::VT_WIDTH, width, 0);
  }
  void add_height(int32_t height) {
    fbb_.AddElement<int32_t>(DepthImageMeasurement::VT_HEIGHT, height, 0);
  }
  DepthImageMeasurementBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  DepthImageMeasurementBuilder &operator=(const DepthImageMeasurementBuilder &);
  flatbuffers::Offset<DepthImageMeasurement> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<DepthImageMeasurement>(end);
    return o;
  }
};

inline flatbuffers::Offset<DepthImageMeasurement> CreateDepthImageMeasurement(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<HeaderData> header = 0,
    flatbuffers::Offset<flatbuffers::Vector<const DepthPixel *>> pixels = 0,
    flatbuffers::Offset<flatbuffers::String> encoding = 0,
    int32_t width = 0,
    int32_t height = 0) {
  DepthImageMeasurementBuilder builder_(_fbb);
  builder_.add_height(height);
  builder_.add_width(width);
  builder_.add_encoding(encoding);
  builder_.add_pixels(pixels);
  builder_.add_header(header);
  return builder_.Finish();
}

inline flatbuffers::Offset<DepthImageMeasurement> CreateDepthImageMeasurementDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<HeaderData> header = 0,
    const std::vector<const DepthPixel *> *pixels = nullptr,
    const char *encoding = nullptr,
    int32_t width = 0,
    int32_t height = 0) {
  return RHMsg::CreateDepthImageMeasurement(
      _fbb,
      header,
      pixels ? _fbb.CreateVector<const DepthPixel *>(*pixels) : 0,
      encoding ? _fbb.CreateString(encoding) : 0,
      width,
      height);
}

inline const RHMsg::DepthImageMeasurement *GetDepthImageMeasurement(const void *buf) {
  return flatbuffers::GetRoot<RHMsg::DepthImageMeasurement>(buf);
}

inline bool VerifyDepthImageMeasurementBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<RHMsg::DepthImageMeasurement>(nullptr);
}

inline void FinishDepthImageMeasurementBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<RHMsg::DepthImageMeasurement> root) {
  fbb.Finish(root);
}

}  // namespace RHMsg

#endif  // FLATBUFFERS_GENERATED_DEPTHIMAGEMEASUREMENT_RHMSG_H_