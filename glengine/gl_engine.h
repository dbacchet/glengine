#pragma once

#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "gl_stock_shaders.h"
#include "gl_prefabs.h"
#include "gl_renderobject.h"

#include <cstdint>
#include <unordered_map>

namespace glengine {

class Mesh;
class Shader;

struct Config {
    uint32_t window_width = 1280;
    uint32_t window_height = 720;
    bool vsync = true;
};

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
    Mesh* create_mesh(uint32_t id);
    /// get mesh by id
    Mesh* get_mesh(uint32_t id);
    /// check if the mesh with the given id exists
    bool has_mesh(uint32_t id) const;

    // prefab meshes
    /// solid box
    Mesh* create_box_mesh(uint32_t id, const math::Vector3f &size={1.0f,1.0f,1.0f});
    /// grid
    Mesh* create_grid_mesh(uint32_t id, float len, float step);

    // /////// //
    // shaders //
    // /////// //

    /// create a new (uninitialized) shader
    Shader* create_shader(uint32_t id);
    /// get shader by id
    Shader* get_shader(uint32_t id);
    /// check if the shader with the given id exists
    bool has_shader(uint32_t id) const;
    /// get stock shader
    Shader* get_stock_shader(StockShader type);

    // ///////////// //
    // renderobjects //
    // ///////////// //
    /// create a new (uninitialized) renderobject
    RenderObject* create_renderobject(uint32_t id);
    /// create a new renderobject, given a mesh and a shader
    RenderObject* create_renderobject(uint32_t id, Mesh *mesh, Shader *shader);
    /// get renderobject by id
    RenderObject* get_renderobject(uint32_t id);
    /// check if the renderobject with the given id exists
    bool has_renderobject(uint32_t id) const;

    // prefabs
    // RenderObject* create_box(uint32_t id, const math::Vector3f &size={1.0f,1.0f,1.0f}, StockShader shader=StockShader::Diffuse);

// protected:
    Context _context;
    Camera _camera;
    CameraManipulator _camera_manipulator;


    std::unordered_map<uint32_t, Mesh*> _meshes;
    std::unordered_map<uint32_t, Shader*> _shaders;
    std::unordered_map<StockShader, Shader*> _stock_shaders;

    std::unordered_map<uint32_t, RenderObject*> _renderobjects;

    void create_stock_shaders();
};

} // namespace glengine
