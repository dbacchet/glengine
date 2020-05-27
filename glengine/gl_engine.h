#pragma once

#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "gl_stock_shaders.h"
#include "gl_prefabs.h"
#include "gl_renderobject.h"
#include "gl_resource_manager.h"
#include "imgui/imgui.h"

#include <cstdint>
#include <unordered_map>
#include <functional>
#include <limits>

namespace glengine {

class Mesh;
class Shader;

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
    ResourceManager& resource_manager() {
        return _resource_manager;
    }

    // ///////////// //
    // renderobjects //
    // ///////////// //
    /// create a new (uninitialized) renderobject
    RenderObject *create_renderobject(ID id);
    /// create a new renderobject, given a mesh and a shader
    RenderObject *create_renderobject(ID id, Mesh *mesh, Shader *shader);
    /// create a new renderobject, given an array of meshes and a shader
    RenderObject *create_renderobject(ID id, const std::vector<Mesh*> &meshes, Shader *shader);
    /// get renderobject by id
    RenderObject *get_renderobject(ID id);
    /// check if the renderobject with the given id exists
    bool has_renderobject(ID id) const;

    // // //
    // UI //
    // // //
    /// add a function to be called during the UI rendering
    void add_ui_function(std::function<void(void)> fun);

    // ///////// //
    // selection //
    // ///////// //
    /// get the current cursor coordinates
    math::Vector2i cursor_pos() const;
    /// get the id of the object at the given (screen) coordinates.
    /// \return object id or NULL_ID if none
    ID object_at_screen_coord(const math::Vector2i &cursor_pos) const;

    // protected:
    Context _context;
    Camera _camera;
    CameraManipulator _camera_manipulator;
    ResourceManager _resource_manager;

    GLuint _g_buffer = 0;              // framebuffer id
    GLuint _gb_color = INVALID_BUFFER; // framebuffer color attachment handle
    GLuint _gb_id = INVALID_BUFFER;    // framebuffer object_id attachment handle
    GLuint _gb_depth = INVALID_BUFFER; // framebuffer depth+stencil attachment handle
    Mesh *_ss_quad = nullptr;
    std::vector<ID> _id_buffer; // buffer containing the id of the object in every pixel

    std::unordered_map<ID, Mesh *> _meshes;

    std::unordered_map<ID, RenderObject *> _renderobjects;

    std::vector<std::function<void(void)>> _ui_functions;

    ID _next_mesh_id = 0;

    void resize_buffers();
};

} // namespace glengine
