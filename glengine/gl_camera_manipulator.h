#pragma once

#include "math/vmath.h"
#include "gl_camera.h"

namespace glengine {

class CameraManipulator {
  public:
    float azimuth() const { return _azimuth; }
    float elevation() const { return _elevation; }
    float distance() const { return _distance; }
    const math::Vector3f &center() const { return _center; }

    float &azimuth() { return _azimuth; }
    float &elevation() { return _elevation; }
    float &distance() { return _distance; }
    math::Vector3f &center() { return _center; }

    // set azimuth/elevation/distance
    CameraManipulator &set_azimuth(float angle);
    CameraManipulator &set_elevation(float angle);
    CameraManipulator &set_distance(float val);
    // increment azimuth/elevation/distance
    CameraManipulator &add_azimuth(float angle);
    CameraManipulator &add_elevation(float angle);
    CameraManipulator &add_distance(float val);
    // camera target position
    CameraManipulator &set_center(const math::Vector3f &pos);
    CameraManipulator &translate(const math::Vector3f &delta);

    bool update(Camera &cam);

  protected:
    float _azimuth = 0;
    float _elevation = 0;
    float _distance = 10.0f;
    float _min_distance = 0.1f;
    math::Vector3f _center = {0, 0, 0};
};
} // namespace glengine
