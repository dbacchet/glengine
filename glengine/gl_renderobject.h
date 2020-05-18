#pragma once

#include "math/vmath.h"
#include "gl_mesh.h"
#include "gl_shader.h"

namespace glengine {

class RenderObject {
  public:
    // //// //
    // data //
    // //// //
    // const std::vector<Vertex> &vertices() const { return _vertices; }
    // const std::vector<uint32_t> &indices() const { return _indices; }
    //
    // std::vector<Vertex> &mutable_vertices() {
    //     _data_dirty = true;
    //     return _vertices;
    // }
    // std::vector<uint32_t> &mutable_indices() {
    //     _data_dirty = true;
    //     return _indices;
    // }
    //
    // RenderObject &set_vertices(const std::vector<Vertex> &vertices) {
    //     _vertices = vertices;
    //     _data_dirty = true;
    //     return *this;
    // }
    // RenderObject &set_vertices(const Vertex *vertices, uint32_t nv) {
    //     _vertices = std::vector<Vertex>(vertices, vertices + nv);
    //     _data_dirty = true;
    //     return *this;
    // }
    // RenderObject &set_indices(const std::vector<uint32_t> indices) {
    //     _indices = indices;
    //     _data_dirty = true;
    //     return *this;
    // }
    // RenderObject &set_indices(const uint32_t *indices, uint32_t ni) {
    //     _indices = std::vector<uint32_t>(indices, indices + ni);
    //     _data_dirty = true;
    //     return *this;
    // }

    // ////////// //
    // attributes //
    // ////////// //
    math::Matrix4f transform() const { return _transform; }
    math::Vector3f scale() const { return {_transform(0, 0), _transform(1, 1), _transform(2, 2)}; }
    math::Vector4f color() const { return _color; }
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
    RenderObject &set_color(const math::Vector4f &color) {
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
    math::Vector4f _color = {0.5f, 0.5f, 0.5f, 1.0f};
    bool _visible = true; ///< visibility flag

    bool _data_dirty = true; ///< flag to mark the data (vertices and indices) as out-of-date
    bool _attr_dirty = true; ///< flag to mark the attributes (transform etc) as out-of-date
};

} // namespace glengine
