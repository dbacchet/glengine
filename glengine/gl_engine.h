#pragma once

#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "gl_stock_shaders.h"
#include "gl_prefabs.h"
#include "gl_renderobject.h"
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

    // ////// //
    // meshes //
    // ////// //

    /// create a new (empty) mesh
    Mesh *create_mesh(ID id);
    /// get mesh by id
    Mesh *get_mesh(ID id);
    /// check if the mesh with the given id exists
    bool has_mesh(ID id) const;

    // prefab meshes
    /// axis gizmo
    Mesh *create_axis_mesh(ID id);
    /// quad that extends -1..1
    Mesh *create_quad_mesh(ID id);
    /// solid box
    Mesh *create_box_mesh(ID id, const math::Vector3f &size = {1.0f, 1.0f, 1.0f});
    /// sphere
    Mesh *create_sphere_mesh(ID id, float radius = 1.0f, uint32_t subdiv = 10);
    /// grid
    Mesh *create_grid_mesh(ID id, float len, float step);

    // /////// //
    // shaders //
    // /////// //

    /// create a new (uninitialized) shader
    Shader *create_shader(ID id);
    /// get shader by id
    Shader *get_shader(ID id);
    /// check if the shader with the given id exists
    bool has_shader(ID id) const;
    /// get stock shader
    Shader *get_stock_shader(StockShader type);

    // ///////////// //
    // renderobjects //
    // ///////////// //
    /// create a new (uninitialized) renderobject
    RenderObject *create_renderobject(ID id);
    /// create a new renderobject, given a mesh and a shader
    RenderObject *create_renderobject(ID id, Mesh *mesh, Shader *shader);
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

    GLuint _g_buffer = 0;              // framebuffer id
    GLuint _gb_color = INVALID_BUFFER; // framebuffer color attachment handle
    GLuint _gb_id = INVALID_BUFFER;    // framebuffer object_id attachment handle
    GLuint _gb_depth = INVALID_BUFFER; // framebuffer depth+stencil attachment handle
    Mesh *_ss_quad = nullptr;
    std::vector<ID> _id_buffer; // buffer containing the id of the object in every pixel

    std::unordered_map<ID, Mesh *> _meshes;
    std::unordered_map<ID, Shader *> _shaders;
    std::unordered_map<StockShader, Shader *> _stock_shaders;

    std::unordered_map<ID, RenderObject *> _renderobjects;

    std::vector<std::function<void(void)>> _ui_functions;

    void create_stock_shaders();

    void resize_buffers();
};

} // namespace glengine
