#pragma once

#include "math/vmath.h"
#include "gl_types.h"
#include "gl_camera.h"
#include "gl_renderer.h"

#include <vector>
#include <set>

namespace glengine {

class Mesh;
class Shader;
class Material;

struct GfxObject {
    Mesh *mesh = nullptr;
    Material *material = nullptr;
};

class RenderObject final {
  public:
    RenderObject(RenderObject *parent = nullptr, ID id = NULL_ID);
    ~RenderObject();

    bool init(Mesh *mesh, Shader *shader);
    bool init(std::vector<Mesh *> meshes, Shader *shader);

    bool draw(Renderer &renderer, const Camera &cam, const math::Matrix4f &parent_tf=math::matrix4_identity<float>());

    // ////////// //
    // scenegraph //
    // ////////// //

    RenderObject *parent() { return _parent; }
    std::set<RenderObject *> &children() { return _children; }
    void add_child(RenderObject *ro);
    /// detach the given child and return a pointer to the orphan renderobject
    RenderObject *detach_child(RenderObject *child);

    // ////////// //
    // attributes //
    // ////////// //
    math::Matrix4f transform() const { return _transform; }
    math::Vector3f scale() const { return {_transform(0, 0), _transform(1, 1), _transform(2, 2)}; }
    bool visible() const { return _visible; }

    RenderObject &set_transform(const math::Matrix4f &tf);
    RenderObject &set_scale(const math::Vector3f &scl);
    RenderObject &set_visible(bool flag);

    // //// //
    // data //
    // //// //

    ID _id = NULL_ID;
    RenderObject *_parent = nullptr;
    std::set<RenderObject *> _children;

    std::vector<Mesh *> _meshes;
    Shader *_shader = nullptr;
    std::vector<GfxObject *> _gfxobjects;

    math::Matrix4f _transform = math::matrix4_identity<float>();
    math::Matrix4f _scale = math::matrix4_identity<float>();
    bool _visible = true; ///< visibility flag
};

} // namespace glengine
