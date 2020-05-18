#pragma once

#include "math/vmath.h"
#include "gl_camera.h"

namespace glengine {

class CameraManipulator {
  public:
    float _azimuth = 0;
    float _elevation = 0;
    float _distance = 10.0f;
    float _min_distance = 0.1f;
    math::Vector3f _center = {0, 0, 0};

    float azimuth() const { return _azimuth; }
    float elevation() const { return _elevation; }
    float distance() const { return _distance; }

    CameraManipulator &set_azimuth(float angle) {
        _azimuth = angle;
        return *this;
    }
    CameraManipulator &add_azimuth(float angle) {
        _azimuth += angle;
        return *this;
    }
    CameraManipulator &set_elevation(float angle) {
        _elevation = angle;
        return *this;
    }
    CameraManipulator &add_elevation(float angle) {
        _elevation += angle;
        return *this;
    }
    CameraManipulator &set_distance(float val) {
        _distance = val;
        if (_distance < _min_distance)
            _distance = _min_distance;
        return *this;
    }
    CameraManipulator &add_distance(float val) {
        _distance += val;
        if (_distance < _min_distance)
            _distance = _min_distance;
        return *this;
    }

    CameraManipulator &set_center(const math::Vector3f &pos) {
        _center = pos;
        return *this;
    };
    CameraManipulator &translate(const math::Vector3f &delta) {
        _center += delta;
        return *this;
    };

    bool update(Camera &cam);
};
} // namespace glengine
