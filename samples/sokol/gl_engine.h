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

    // ///////// //
    // materials //
    // ///////// //
    /// create a material
    template <typename MtlT>
    MtlT *create_material(sg_primitive_type primitive, sg_index_type idx_type = SG_INDEXTYPE_NONE) {
        MtlT *mtl = new MtlT();
        if (mtl && mtl->init(_resource_manager, primitive, idx_type)) {
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
