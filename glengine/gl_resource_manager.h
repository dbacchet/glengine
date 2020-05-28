#pragma once

#include "gl_types.h"
#include "gl_mesh.h"
#include "gl_shader.h"
#include "gl_stock_shaders.h"
#include "gl_texture.h"

#include <cstdint>
#include <unordered_map>

namespace glengine {

// class Resource {
//   public:
//     ID id = NULL_ID;
//     std::string name = "";
//
//   protected:
//     Resource(ID id_, const std::string &name_)
//     : id(id_)
//     , name(name_) {}
// };

class ResourceManager {
  public:
    virtual ~ResourceManager();

    bool init();

    bool terminate();

    // /////// //
    // shaders //
    // /////// //

    /// create a new (uninitialized) shader
    Shader *create_shader();
    /// get shader by id
    Shader *get_shader(ID id);
    /// check if the shader with the given id exists
    bool has_shader(ID id) const;
    /// get stock shader
    Shader *get_stock_shader(StockShader type);

    // //////// //
    // textures //
    // //////// //

    /// create a new (uninitialized) texture
    Texture *create_texture();
    /// get texture by id
    Texture *get_texture(ID id);
    /// check if the texture with the given id exists
    bool has_texture(ID id) const;
    /// create a texture from file
    Texture *create_texture_from_file(const char *filename);
    Texture *create_texture_from_data(uint32_t width, uint32_t height, uint8_t channels, const uint8_t *data);

    // ////// //
    // meshes //
    // ////// //

    /// create a new (empty) mesh
    Mesh *create_mesh();
    /// get mesh by id
    Mesh *get_mesh(ID id);
    /// check if the mesh with the given id exists
    bool has_mesh(ID id) const;
    /// create a mesh from file
    std::vector<Mesh *> create_mesh_from_file(const char *filename);

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
    Mesh *create_grid_mesh(float len, float step);

  protected:
    std::unordered_map<ID, Shader *> _shaders;
    std::unordered_map<StockShader, Shader *> _stock_shaders;
    std::unordered_map<ID, Texture *> _textures;
    std::unordered_map<ID, Mesh *> _meshes;

    ID _next_shader_id = 1;
    ID _next_texture_id = 1;
    ID _next_mesh_id = 1;

    void create_stock_shaders();
};

} // namespace glengine
