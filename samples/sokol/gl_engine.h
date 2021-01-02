#pragma once

#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "gl_resource_manager.h"
#include "gl_object.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace glengine {

struct State;

class GLEngine {
  public:
    virtual ~GLEngine();

    // ////// //
    // engine //
    // ////// //

    bool init(const Config &config = {});

    bool render();

    bool terminate();

    /// get resource manager
    ResourceManager &resource_manager() { return _resource_manager; }

    // /////// //
    // objects //
    // /////// //
    /// create a new (empty) object
    Object *create_object(Object *parent = nullptr, ID id = NULL_ID);
    /// create a new object and add the given renderable
    Object *create_object(const Renderable &renderable, Object *parent = nullptr, ID id = NULL_ID);
    /// create a new object, given an array of renderables
    Object *create_object(const std::vector<Renderable> &renderables, Object *parent = nullptr, ID id = NULL_ID);

    // ////// //
    // meshes //
    // ////// //
    /// create a new (empty) mesh
    /// the mesh will _not_ be initialized, and the user is responsible for that
    Mesh *create_mesh();
    /// create a new mesh given vertices and (optionally) indices
    /// the mesh will be initialized, and the given usage (immutable, dynamic, stream) set
    Mesh *create_mesh(const std::vector<Vertex> &vertices_, const std::vector<uint32_t> &indices_={}, sg_usage usage = SG_USAGE_IMMUTABLE);
    // prefab meshes
    /// axis gizmo
    Mesh *create_axis_mesh();
    /// quad that extends -1..1
    Mesh *create_quad_mesh();
    /// solid box
    Mesh *create_box_mesh(const math::Vector3f &size = {1.0f, 1.0f, 1.0f});
    /// sphere
    Mesh *create_sphere_mesh(float radius = 1.0f, uint32_t subdiv = 10);
    /// grid
    Mesh *create_grid_mesh(float len = 100.0f, float step = 5.0f);

    // ///////// //
    // materials //
    // ///////// //
    /// create a material
    template <typename MtlT>
    MtlT *create_material(sg_primitive_type primitive, sg_index_type idx_type = SG_INDEXTYPE_NONE) {
        MtlT *mtl = new MtlT();
        if (mtl && mtl->init(_resource_manager, primitive, idx_type)) {
            _resource_manager.register_material(mtl);
            return mtl;
        } else {
            delete mtl;
            return nullptr;
        }
    }

    // // //
    // UI //
    // // //
    /// add a function to be called during the UI rendering
    void add_ui_function(std::function<void(void)> fun);

    // protected:
    Config _config;
    Context _context;
    Camera _camera;
    CameraManipulator _camera_manipulator;
    ResourceManager _resource_manager;

    Object *_root = nullptr;

    uint64_t _curr_time = 0;
    State *_state = nullptr; ///< persistent state needed by the renderer

    std::vector<std::function<void(void)>> _ui_functions;
};

} // namespace glengine
