#pragma once

#include "math/vmath.h"
#include "gl_context.h"

#include <cstdint>

namespace glengine {

/// @class Camera
/// @brief component implementing a camera
/// @details this component implements the abstaction of a camera, used by the engine
/// to set the projection and relative transformation matrix.\n
/// The values leftPlane, rightPlane, topPlane, bottomPlane represent the coordinates of
/// the cutting planes intersecting with the nearPlane (in orthographic mode these values
/// also correspond to the intersection with the farPlane, because of the parallel projection)
class Camera {
  public:
    enum class Projection {
        Orthographic, ///< orthographic camera
        Perspective,  ///< perspective camera
        ProjectionNum ///< number of camera modes
    };

    /// destructor
    virtual ~Camera();

    /// update internal state (ratio, projection matrix, etc.)
    bool update(uint32_t width, uint32_t height);

    /// setup ortho projection
    void set_ortho(float nearpl, float farpl, float left, float right, float bottom, float top) {
        _mode = Camera::Projection::Orthographic;
        _near_plane = nearpl;
        _far_plane = farpl;
        _left_plane = left;
        _right_plane = right;
        _bottom_plane = bottom;
        _top_plane = top;
    }

    /// setup perspective projection
    void set_perspective(float nearpl, float farpl, float fov) {
        _mode = Camera::Projection::Perspective;
        _near_plane = nearpl;
        _far_plane = farpl;
        _fov = fov;
    }

    void set_transform(const math::Matrix4f &t) {
        _transform = t;
        _inverse_transform = math::inverse(t);
    }

    // get current projection/transformation matrices
    const math::Matrix4f &transform() const { return _transform; }
    const math::Matrix4f &inverse_transform() const { return _inverse_transform; }
    const math::Matrix4f &projection() const { return _projection; }

    float near_plane() const { return _near_plane; }
    float far_plane() const { return _far_plane; }

  private:
    Projection _mode;    ///< projection mode (orthographic or perspective)
    float _fov;          ///< field-of-view angle (only used in perspective mode)
    float _aspect_ratio; ///< aspect ratio (width/height)
    float _left_plane;   ///< left plane coord (overwritten if fov or aspect_ratio are used)
    float _right_plane;  ///< right plane coord (overwritten if fov or aspect_ratio are used)
    float _bottom_plane; ///< bottom plane coord (overwritten if fov or aspect_ratio are used)
    float _top_plane;    ///< top plane coord (overwritten if fov or aspect_ratio are used)
    float _near_plane;   ///< near plane distance (must be >0 in perspective mode)
    float _far_plane;    ///< far plane distance

    math::Matrix4f _projection;        ///< current projection matrix
    math::Matrix4f _transform;         ///< position/rotation associated transformation
    math::Matrix4f _inverse_transform; ///< inverse of the transformation matrix

    /// create ortographic projection
    bool create_orthographic_proj(void);
    /// create perspective projection
    bool create_perspective_proj(void);

    /// basic initialization
    void init(void);
};

// ///////////////////////////// //
//        IMPLEMENTATION         //
// ///////////////////////////// //

// destructor
inline Camera::~Camera() {}

// update projection matrix
inline bool Camera::update(uint32_t width, uint32_t height) {
    _aspect_ratio = float(width) / height;
    if (_mode == Projection::Perspective)
        return create_perspective_proj();
    if (_mode == Projection::Orthographic)
        return create_orthographic_proj();
    return false;
}

// create ortographic projection
inline bool Camera::create_orthographic_proj(void) {
    _mode = Projection::Orthographic;
    // compensate with aspect ratio
    float w = _right_plane - _left_plane;
    float co = (_right_plane + _left_plane) / 2.0f;
    float h = _top_plane - _bottom_plane;
    float cv = (_top_plane + _bottom_plane) / 2.0f;
    if ((w / h) >= _aspect_ratio) {
        h = w / _aspect_ratio;
        _top_plane = cv + h / 2.0f;
        _bottom_plane = cv - h / 2.0f;
    } else {
        w = h * _aspect_ratio;
        _left_plane = co - w / 2.0f;
        _right_plane = co + w / 2.0f;
    }
    // create projection matrix
    float a = (_right_plane - _left_plane);
    float b = (_top_plane - _bottom_plane);
    float c = (_far_plane - _near_plane);
    float tx = -(_right_plane + _left_plane) / a;
    float ty = -(_top_plane + _bottom_plane) / b;
    float tz = -(_far_plane + _near_plane) / c;
    _projection = math::matrix4_identity<float>();
    _projection(0, 0) = 2.0f / a;
    _projection(1, 1) = 2.0f / b;
    _projection(2, 2) = -2.0f / c;
    _projection(0, 3) = tx;
    _projection(1, 3) = ty;
    _projection(2, 3) = tz;
    return true;
}

// create perspective projection
inline bool Camera::create_perspective_proj(void) {
    _mode = Projection::Perspective;
    // create left, right, top, bottom using fov+aspect ratio
    if (_near_plane < 1E-6f)
        _near_plane = 1E-6f;
    float h = _near_plane * std::tan(_fov / 2.0f);
    float w = h * _aspect_ratio;
    _top_plane = h;
    _bottom_plane = -h;
    _right_plane = w;
    _left_plane = -w;
    // fill projection matrix
    float a = (_right_plane - _left_plane);
    float b = (_top_plane - _bottom_plane);
    float c = (_far_plane - _near_plane);
    float A = (_right_plane + _left_plane) / a;
    float B = (_top_plane + _bottom_plane) / b;
    float C = -(_far_plane + _near_plane) / c;
    float D = -(2 * _far_plane * _near_plane) / c;
    _projection = _projection * 0.0f;
    _projection(0, 0) = 2.0f * _near_plane / a;
    _projection(1, 1) = 2.0f * _near_plane / b;
    _projection(0, 2) = A;
    _projection(1, 2) = B;
    _projection(2, 2) = C;
    _projection(3, 2) = -1.0f;
    _projection(2, 3) = D;

    return true;
}

// basic initialization
inline void Camera::init(void) {
    // default values
    _mode = Projection::Perspective;
    _fov = 45.0f / 180.0f * M_PI;
    _aspect_ratio = 16.0f / 9.0f;
    _projection = math::matrix4_identity<float>();
    _left_plane = -50.0f;
    _right_plane = 50.0f;
    _bottom_plane = -50.0f;
    _top_plane = 50.0f;
    _near_plane = 1.0f;
    _far_plane = 100.0f;
    // default to ortho
    create_orthographic_proj();
}

} // namespace glengine
