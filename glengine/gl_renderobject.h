#pragma once

#include "math/vmath.h"
#include "gl_types.h"
#include "gl_mesh.h"
#include "gl_shader.h"

namespace glengine {

class RenderObject {
  public:

    bool init(Mesh *mesh, Shader *shader) {
        if (!mesh || !shader) {
            return false;
        }
        _mesh = mesh;
        _shader = shader;
        return true;
    }

    bool draw(const Camera &cam) {
        if (_visible) {
            _shader->activate();
            _shader->set_uniform_model(_transform * _scale);
            _shader->set_uniform_view(cam.inverse_transform());
            _shader->set_uniform_projection(cam.projection());
            _shader->set_uniform_color(_color);
            _shader->set_uniform_light0_pos(math::Vector3f(100,100,100));
            _mesh->draw();
        }
        return true;
    }
    // ////////// //
    // attributes //
    // ////////// //
    math::Matrix4f transform() const { return _transform; }
    math::Vector3f scale() const { return {_transform(0, 0), _transform(1, 1), _transform(2, 2)}; }
    glengine::Color color() const { return _color; }
    bool visible() const { return _visible; }

    RenderObject &set_transform(const math::Matrix4f &tf) {
        _transform = tf;
        _attr_dirty = true; // mark the attributes as changed
        return *this;
    }
    RenderObject &set_scale(const math::Vector3f &scl) {
        _scale(0, 0) = scl[0];
        _scale(1, 1) = scl[1];
        _scale(2, 2) = scl[2];
        _attr_dirty = true; // mark the attributes as changed
        return *this;
    }
    RenderObject &set_color(const glengine::Color &color) {
        _color = color;
        _attr_dirty = true; // mark the attributes as changed
        return *this;
    }
    RenderObject &set_visible(bool flag) {
        _visible = flag;
        _attr_dirty = true; // mark the attributes as changed
        return *this;
    }

    Mesh *_mesh = nullptr;
    Shader *_shader = nullptr;

    math::Matrix4f _transform = math::matrix4_identity<float>();
    math::Matrix4f _scale = math::matrix4_identity<float>();
    glengine::Color _color = {100,100,100,255};
    bool _visible = true; ///< visibility flag

    bool _data_dirty = true; ///< flag to mark the data (vertices and indices) as out-of-date
    bool _attr_dirty = true; ///< flag to mark the attributes (transform etc) as out-of-date
};

} // namespace glengine
