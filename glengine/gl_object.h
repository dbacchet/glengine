#pragma once

#include "math/vmath.h"
#include "gl_types.h"
#include "gl_camera.h"
#include "gl_renderable.h"

#include <vector>
#include <set>

namespace glengine {

class Mesh;
class Shader;
class Material;
class Renderer;

class Object final {
  public:
    Object(Object *parent = nullptr, ID id = NULL_ID);
    ~Object();

    bool init(Mesh *mesh, Shader *shader);
    bool init(std::vector<Mesh *> meshes, Shader *shader);
    bool init(const std::vector<Renderable> &renderables);
    bool add_renderable(const Renderable *r, uint32_t num);

    bool draw(Renderer &renderer, const Camera &cam, const math::Matrix4f &parent_tf=math::matrix4_identity<float>());

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
    math::Vector3f scale() const { return {_transform(0, 0), _transform(1, 1), _transform(2, 2)}; }
    bool visible() const { return _visible; }

    Object &set_transform(const math::Matrix4f &tf);
    Object &set_scale(const math::Vector3f &scl);
    Object &set_visible(bool flag);

    // //// //
    // data //
    // //// //

    ID _id = NULL_ID;
    Object *_parent = nullptr;
    std::set<Object *> _children;

    std::vector<Mesh *> _meshes;
    Shader *_shader = nullptr;
    std::vector<Renderable> _renderables;

    math::Matrix4f _transform = math::matrix4_identity<float>();
    math::Matrix4f _scale = math::matrix4_identity<float>();
    bool _visible = true; ///< visibility flag
};

} // namespace glengine
