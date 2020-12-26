#include "gl_camera_manipulator.h"
#include "math/vmath.h"

namespace glengine {

    CameraManipulator &CameraManipulator::set_azimuth(float angle) {
        _azimuth = angle;
        return *this;
    }
    CameraManipulator &CameraManipulator::add_azimuth(float angle) {
        _azimuth += angle;
        return *this;
    }
    CameraManipulator &CameraManipulator::set_elevation(float angle) {
        _elevation = angle;
        return *this;
    }
    CameraManipulator &CameraManipulator::add_elevation(float angle) {
        _elevation += angle;
        return *this;
    }
    CameraManipulator &CameraManipulator::set_distance(float val) {
        _distance = val;
        if (_distance < _min_distance)
            _distance = _min_distance;
        return *this;
    }
    CameraManipulator &CameraManipulator::add_distance(float val) {
        _distance += val;
        if (_distance < _min_distance)
            _distance = _min_distance;
        return *this;
    }

    CameraManipulator &CameraManipulator::set_center(const math::Vector3f &pos) {
        _center = pos;
        return *this;
    };
    CameraManipulator &CameraManipulator::translate(const math::Vector3f &delta) {
        _center += delta;
        return *this;
    };
bool CameraManipulator::update(Camera &cam) {
    math::Matrix4f center = math::create_translation<float>(_center);
    math::Matrix4f azim =
        math::create_transformation<float>({0, 0, 0}, math::quat_from_euler_321(0.0f, 0.0f, _azimuth));
    math::Matrix4f elev =
        math::create_transformation<float>({0, 0, 0}, math::quat_from_euler_321(_elevation, 0.0f, 0.0f));
    math::Matrix4f dist = math::create_translation<float>({0, 0, _distance});
    cam.set_transform(center * azim * elev * dist);
    return true;
}
} // namespace glengine
