// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RADARMEASUREMENT_RHMSG_H_
#define FLATBUFFERS_GENERATED_RADARMEASUREMENT_RHMSG_H_

#include "flatbuffers/flatbuffers.h"

#include "header_generated.h"

namespace RHMsg {

struct RadarObject;

struct RadarMeasurement;

enum RadarObjectStatus {
  RadarObjectStatus_NEW = 0,
  RadarObjectStatus_TRACKED = 1,
  RadarObjectStatus_MIN = RadarObjectStatus_NEW,
  RadarObjectStatus_MAX = RadarObjectStatus_TRACKED
};

inline RadarObjectStatus (&EnumValuesRadarObjectStatus())[2] {
  static RadarObjectStatus values[] = {
    RadarObjectStatus_NEW,
    RadarObjectStatus_TRACKED
  };
  return values;
}

inline const char **EnumNamesRadarObjectStatus() {
  static const char *names[] = {
    "NEW",
    "TRACKED",
    nullptr
  };
  return names;
}

inline const char *EnumNameRadarObjectStatus(RadarObjectStatus e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesRadarObjectStatus()[index];
}

MANUALLY_ALIGNED_STRUCT(4) RadarObject FLATBUFFERS_FINAL_CLASS {
 private:
  float yaw_;
  float range_;
  float power_db_;
  float range_rate_;
  float lat_rate_;
  int32_t id_;
  int8_t status_;
  int8_t padding0__;  int16_t padding1__;

 public:
  RadarObject() {
    memset(this, 0, sizeof(RadarObject));
  }
  RadarObject(const RadarObject &_o) {
    memcpy(this, &_o, sizeof(RadarObject));
  }
  RadarObject(float _yaw, float _range, float _power_db, float _range_rate, float _lat_rate, int32_t _id, RadarObjectStatus _status)
      : yaw_(flatbuffers::EndianScalar(_yaw)),
        range_(flatbuffers::EndianScalar(_range)),
        power_db_(flatbuffers::EndianScalar(_power_db)),
        range_rate_(flatbuffers::EndianScalar(_range_rate)),
        lat_rate_(flatbuffers::EndianScalar(_lat_rate)),
        id_(flatbuffers::EndianScalar(_id)),
        status_(flatbuffers::EndianScalar(static_cast<int8_t>(_status))),
        padding0__(0),
        padding1__(0) {
    (void)padding0__;    (void)padding1__;
  }
  float yaw() const {
    return flatbuffers::EndianScalar(yaw_);
  }
  float range() const {
    return flatbuffers::EndianScalar(range_);
  }
  float power_db() const {
    return flatbuffers::EndianScalar(power_db_);
  }
  float range_rate() const {
    return flatbuffers::EndianScalar(range_rate_);
  }
  float lat_rate() const {
    return flatbuffers::EndianScalar(lat_rate_);
  }
  int32_t id() const {
    return flatbuffers::EndianScalar(id_);
  }
  RadarObjectStatus status() const {
    return static_cast<RadarObjectStatus>(flatbuffers::EndianScalar(status_));
  }
};
STRUCT_END(RadarObject, 28);

struct RadarMeasurement FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_HEADER = 4,
    VT_RADAR_SENSOR = 6
  };
  const HeaderData *header() const {
    return GetPointer<const HeaderData *>(VT_HEADER);
  }
  const flatbuffers::Vector<const RadarObject *> *radar_sensor() const {
    return GetPointer<const flatbuffers::Vector<const RadarObject *> *>(VT_RADAR_SENSOR);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_HEADER) &&
           verifier.VerifyTable(header()) &&
           VerifyOffset(verifier, VT_RADAR_SENSOR) &&
           verifier.Verify(radar_sensor()) &&
           verifier.EndTable();
  }
};

struct RadarMeasurementBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_header(flatbuffers::Offset<HeaderData> header) {
    fbb_.AddOffset(RadarMeasurement::VT_HEADER, header);
  }
  void add_radar_sensor(flatbuffers::Offset<flatbuffers::Vector<const RadarObject *>> radar_sensor) {
    fbb_.AddOffset(RadarMeasurement::VT_RADAR_SENSOR, radar_sensor);
  }
  RadarMeasurementBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RadarMeasurementBuilder &operator=(const RadarMeasurementBuilder &);
  flatbuffers::Offset<RadarMeasurement> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<RadarMeasurement>(end);
    return o;
  }
};

inline flatbuffers::Offset<RadarMeasurement> CreateRadarMeasurement(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<HeaderData> header = 0,
    flatbuffers::Offset<flatbuffers::Vector<const RadarObject *>> radar_sensor = 0) {
  RadarMeasurementBuilder builder_(_fbb);
  builder_.add_radar_sensor(radar_sensor);
  builder_.add_header(header);
  return builder_.Finish();
}

inline flatbuffers::Offset<RadarMeasurement> CreateRadarMeasurementDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<HeaderData> header = 0,
    const std::vector<const RadarObject *> *radar_sensor = nullptr) {
  return RHMsg::CreateRadarMeasurement(
      _fbb,
      header,
      radar_sensor ? _fbb.CreateVector<const RadarObject *>(*radar_sensor) : 0);
}

inline const RHMsg::RadarMeasurement *GetRadarMeasurement(const void *buf) {
  return flatbuffers::GetRoot<RHMsg::RadarMeasurement>(buf);
}

inline bool VerifyRadarMeasurementBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<RHMsg::RadarMeasurement>(nullptr);
}

inline void FinishRadarMeasurementBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<RHMsg::RadarMeasurement> root) {
  fbb.Finish(root);
}

}  // namespace RHMsg

#endif  // FLATBUFFERS_GENERATED_RADARMEASUREMENT_RHMSG_H_