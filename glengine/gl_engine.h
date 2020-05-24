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
    Mesh* create_mesh(ID id);
    /// get mesh by id
    Mesh* get_mesh(ID id);
    /// check if the mesh with the given id exists
    bool has_mesh(ID id) const;

    // prefab meshes
    /// axis gizmo
    Mesh* create_axis_mesh(ID id);
    /// quad that extends -1..1
    Mesh* create_quad_mesh(ID id);
    /// solid box
    Mesh* create_box_mesh(ID id, const math::Vector3f &size={1.0f,1.0f,1.0f});
    /// sphere
    Mesh* create_sphere_mesh(ID id, float radius=1.0f, uint32_t subdiv=10);
    /// grid
    Mesh* create_grid_mesh(ID id, float len, float step);

    // /////// //
    // shaders //
    // /////// //

    /// create a new (uninitialized) shader
    Shader* create_shader(ID id);
    /// get shader by id
    Shader* get_shader(ID id);
    /// check if the shader with the given id exists
    bool has_shader(ID id) const;
    /// get stock shader
    Shader* get_stock_shader(StockShader type);

    // ///////////// //
    // renderobjects //
    // ///////////// //
    /// create a new (uninitialized) renderobject
    RenderObject* create_renderobject(ID id);
    /// create a new renderobject, given a mesh and a shader
    RenderObject* create_renderobject(ID id, Mesh *mesh, Shader *shader);
    /// get renderobject by id
    RenderObject* get_renderobject(ID id);
    /// check if the renderobject with the given id exists
    bool has_renderobject(ID id) const;

    // prefabs
    // RenderObject* create_box(uint32_t id, const math::Vector3f &size={1.0f,1.0f,1.0f}, StockShader shader=StockShader::Diffuse);

    // // //
    // UI //
    // // //
    /// add a function to be called during the UI rendering
    void add_ui_function(std::function<void(void)> fun);

// protected:
    Context _context;
    Camera _camera;
    CameraManipulator _camera_manipulator;

    GLuint _g_buffer = 0; // framebuffer id
    GLuint _gb_color = 0; // framebuffer color attachment handle
    GLuint _gb_id = 0;    // framebuffer object_id attachment handle
    Mesh *_ss_quad = nullptr;


    std::unordered_map<ID, Mesh*> _meshes;
    std::unordered_map<ID, Shader*> _shaders;
    std::unordered_map<StockShader, Shader*> _stock_shaders;

    std::unordered_map<ID, RenderObject*> _renderobjects;

    std::vector<std::function<void(void)>> _ui_functions;

    void create_stock_shaders();
};

} // namespace glengine
