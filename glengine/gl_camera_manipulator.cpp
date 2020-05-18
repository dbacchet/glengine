#include "gl_camera_manipulator.h"
#include "math/vmath.h"

namespace glengine {

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
