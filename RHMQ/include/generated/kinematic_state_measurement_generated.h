// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_KINEMATICSTATEMEASUREMENT_RHMSG_H_
#define FLATBUFFERS_GENERATED_KINEMATICSTATEMEASUREMENT_RHMSG_H_

#include "flatbuffers/flatbuffers.h"

#include "common_generated.h"
#include "header_generated.h"

namespace RHMsg {

struct KinematicStateMeasurement;

struct KinematicStateMeasurement FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_HEADER = 4,
    VT_UUID = 6,
    VT_LOCATION = 8,
    VT_ORIENTATION = 10,
    VT_VELOCITY = 12,
    VT_ANGULAR_VELOCITY = 14,
    VT_LOCATION_WGS84 = 16
  };
  const HeaderData *header() const {
    return GetPointer<const HeaderData *>(VT_HEADER);
  }
  const UUID *uuid() const {
    return GetStruct<const UUID *>(VT_UUID);
  }
  const Vec3 *location() const {
    return GetStruct<const Vec3 *>(VT_LOCATION);
  }
  const Quat4 *orientation() const {
    return GetStruct<const Quat4 *>(VT_ORIENTATION);
  }
  const Vec3 *velocity() const {
    return GetStruct<const Vec3 *>(VT_VELOCITY);
  }
  const Vec3 *angular_velocity() const {
    return GetStruct<const Vec3 *>(VT_ANGULAR_VELOCITY);
  }
  const Vec3 *location_wgs84() const {
    return GetStruct<const Vec3 *>(VT_LOCATION_WGS84);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_HEADER) &&
           verifier.VerifyTable(header()) &&
           VerifyField<UUID>(verifier, VT_UUID) &&
           VerifyField<Vec3>(verifier, VT_LOCATION) &&
           VerifyField<Quat4>(verifier, VT_ORIENTATION) &&
           VerifyField<Vec3>(verifier, VT_VELOCITY) &&
           VerifyField<Vec3>(verifier, VT_ANGULAR_VELOCITY) &&
           VerifyField<Vec3>(verifier, VT_LOCATION_WGS84) &&
           verifier.EndTable();
  }
};

struct KinematicStateMeasurementBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_header(flatbuffers::Offset<HeaderData> header) {
    fbb_.AddOffset(KinematicStateMeasurement::VT_HEADER, header);
  }
  void add_uuid(const UUID *uuid) {
    fbb_.AddStruct(KinematicStateMeasurement::VT_UUID, uuid);
  }
  void add_location(const Vec3 *location) {
    fbb_.AddStruct(KinematicStateMeasurement::VT_LOCATION, location);
  }
  void add_orientation(const Quat4 *orientation) {
    fbb_.AddStruct(KinematicStateMeasurement::VT_ORIENTATION, orientation);
  }
  void add_velocity(const Vec3 *velocity) {
    fbb_.AddStruct(KinematicStateMeasurement::VT_VELOCITY, velocity);
  }
  void add_angular_velocity(const Vec3 *angular_velocity) {
    fbb_.AddStruct(KinematicStateMeasurement::VT_ANGULAR_VELOCITY, angular_velocity);
  }
  void add_location_wgs84(const Vec3 *location_wgs84) {
    fbb_.AddStruct(KinematicStateMeasurement::VT_LOCATION_WGS84, location_wgs84);
  }
  KinematicStateMeasurementBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  KinematicStateMeasurementBuilder &operator=(const KinematicStateMeasurementBuilder &);
  flatbuffers::Offset<KinematicStateMeasurement> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<KinematicStateMeasurement>(end);
    return o;
  }
};

inline flatbuffers::Offset<KinematicStateMeasurement> CreateKinematicStateMeasurement(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<HeaderData> header = 0,
    const UUID *uuid = 0,
    const Vec3 *location = 0,
    const Quat4 *orientation = 0,
    const Vec3 *velocity = 0,
    const Vec3 *angular_velocity = 0,
    const Vec3 *location_wgs84 = 0) {
  KinematicStateMeasurementBuilder builder_(_fbb);
  builder_.add_location_wgs84(location_wgs84);
  builder_.add_angular_velocity(angular_velocity);
  builder_.add_velocity(velocity);
  builder_.add_orientation(orientation);
  builder_.add_location(location);
  builder_.add_uuid(uuid);
  builder_.add_header(header);
  return builder_.Finish();
}

inline const RHMsg::KinematicStateMeasurement *GetKinematicStateMeasurement(const void *buf) {
  return flatbuffers::GetRoot<RHMsg::KinematicStateMeasurement>(buf);
}

inline bool VerifyKinematicStateMeasurementBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<RHMsg::KinematicStateMeasurement>(nullptr);
}

inline void FinishKinematicStateMeasurementBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<RHMsg::KinematicStateMeasurement> root) {
  fbb.Finish(root);
}

}  // namespace RHMsg

#endif  // FLATBUFFERS_GENERATED_KINEMATICSTATEMEASUREMENT_RHMSG_H_