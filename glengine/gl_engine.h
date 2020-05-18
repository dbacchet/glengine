#pragma once

#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"

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
    Mesh& create_mesh(uint32_t id);
    /// get mesh by id
    Mesh& get_mesh(uint32_t id);
    /// check if the mesh with the given id exists
    bool has_mesh(uint32_t id) const;

    // ////// //
    // shaders //
    // ////// //

    /// create a new (uninitialized) shader
    Shader& create_shader(uint32_t id);
    /// get shader by id
    Shader& get_shader(uint32_t id);
    /// check if the shader with the given id exists
    bool has_shader(uint32_t id) const;

// protected:
    Context _context;
    Camera _camera;
    CameraManipulator _camera_manipulator;

    std::unordered_map<uint32_t, Mesh*> _meshes;
    std::unordered_map<uint32_t, Shader*> _shaders;
};

} // namespace glengine
