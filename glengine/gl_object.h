#pragma once

#include "math/vmath.h"
#include "gl_types.h"
#include "gl_camera.h"
#include "gl_renderable.h"

#include <vector>
#include <set>

namespace glengine {

class Object final {
  public:
    Object(Object *parent = nullptr, ID id = NULL_ID);
    ~Object();

    bool init(const std::vector<Renderable> &renderables);
    bool add_renderable(const Renderable *r, uint32_t num);

    /// update both buffer data and bindings for _all_ renderables with a dynamic mesh.
    /// Note: this is potentially very expensive: consider update_bindings() in case the buffers have not been changed
    void update();
    /// update the bindings for buffers and samplers
    void update_bindings();

    // ////////// //
    // scenegraph //
    // ////////// //

    Object *parent() { return _parent; }
    std::set<Object *> &children() { return _children; }
    void add_child(Object *ro);
    /// detach the given child and return a pointer to the orphan renderobject
    Object *detach_child(Object *child);

    // ////////// //
    // attributes //
    // ////////// //
    math::Matrix4f transform() const { return _transform; }
    math::Vector3f scale() const { return {_scale(0, 0), _scale(1, 1), _scale(2, 2)}; }
    bool visible() const { return _visible; }

    Object &set_transform(const math::Matrix4f &tf);
    Object &set_scale(const math::Vector3f &scl);
    Object &set_visible(bool flag);

    bool draw(const Camera &cam, const math::Matrix4f &parent_tf);

    // //// //
    // data //
    // //// //

    ID _id = NULL_ID;
    Object *_parent = nullptr;
    std::set<Object *> _children;

    std::vector<Renderable> _renderables;

    math::Matrix4f _transform = math::matrix4_identity<float>();
    math::Matrix4f _scale = math::matrix4_identity<float>();
    bool _visible = true; ///< visibility flag
};

} // namespace glengine
