#pragma once

#include "gl_types.h"
#include "gl_mesh.h"
#include "gl_shader.h"
#include "gl_stock_shaders.h"
#include "gl_texture.h"
#include "gl_object.h"

#include <cstdint>
#include <set>
#include <unordered_map>

namespace glengine {

class ResourceManager {
  public:
    virtual ~ResourceManager();

    bool init();

    bool terminate();

    // /////// //
    // shaders //
    // /////// //

    /// create a new (uninitialized) shader
    Shader *create_shader(const char *name);
    /// get shader by id (1:1 mapping)
    Shader *get_shader(ID id);
    /// get shader by name (1:N mapping)
    std::set<Shader*> get_shader(const char *name);
    /// check if the shader with the given id exists
    bool has_shader(ID id) const;
    /// check if the shader with the given name exists
    bool has_shader(const char *name) const;
    /// get stock shader
    Shader *get_stock_shader(StockShader type);

    // //////// //
    // textures //
    // //////// //

    /// create a new (uninitialized) texture
    Texture *create_texture(const char *name);
    /// get texture by id
    Texture *get_texture(ID id);
    /// get texture by name
    std::set<Texture*> get_texture(const char *name);
    /// check if the texture with the given id exists
    bool has_texture(ID id) const;
    /// check if the texture with the given name exists
    bool has_texture(const char *name) const;
    /// create a texture from file
    Texture *create_texture_from_file(const char *filename);
    Texture *create_texture_from_data(const char *name, uint32_t width, uint32_t height, uint8_t channels, const uint8_t *data);

    // ///////// //
    // materials //
    // ///////// //

    /// create a new (default) material
    Material *create_material(const char *name, Shader *shader);
    Material *create_material(const char *name, StockShader type);
    /// get material by id
    Material *get_material(ID id);
    /// get material by name
    std::set<Material*> get_material(const char *name);
    /// check if the material with the given id exists
    bool has_material(ID) const;
    /// check if the material with the given name exists
    bool has_material(const char *name) const;

    // ////// //
    // meshes //
    // ////// //

    /// create a new (empty) mesh
    Mesh *create_mesh(const char *name);
    /// get mesh by id
    Mesh *get_mesh(ID id);
    /// get mesh by name
    std::set<Mesh*> get_mesh(const char *name);
    /// check if the mesh with the given id exists
    bool has_mesh(ID id) const;
    /// check if the mesh with the given name exists
    bool has_mesh(const char *name) const;
    /// create a mesh from file
    std::vector<Renderable> create_mesh_from_file(const char *filename);

    // prefab meshes
    /// axis gizmo
    Mesh *create_axis_mesh(const char *name = "axis_gizmo");
    /// quad that extends -1..1
    Mesh *create_quad_mesh(const char *name = "quad");
    /// solid box
    Mesh *create_box_mesh(const char *name = "box", const math::Vector3f &size = {1.0f, 1.0f, 1.0f});
    /// sphere
    Mesh *create_sphere_mesh(const char *name = "sphere", float radius = 1.0f, uint32_t subdiv = 10);
    /// grid
    Mesh *create_grid_mesh(const char *name = "grid", float len = 100.0f, float step = 5.0f);
    /// polyline with width
    Mesh *create_polyline_mesh(const char *name, const math::Vector3f *pts, uint32_t npts, float width = 1.0f, const math::Vector3f &up = {0.0f,0.0f,1.0f});

  protected:
    std::unordered_map<ID, Shader *> _shaders;
    std::unordered_map<StockShader, Shader *> _stock_shaders;
    std::unordered_map<ID, Texture *> _textures;
    std::unordered_map<ID, Material *> _materials;
    std::unordered_map<ID, Mesh *> _meshes;

    std::unordered_map<std::string, std::set<Shader*>> _shader_lookup;
    std::unordered_map<std::string, std::set<Texture*>> _texture_lookup;
    std::unordered_map<std::string, std::set<Material*>> _material_lookup;
    std::unordered_map<std::string, std::set<Mesh*>> _mesh_lookup;

    ID _next_shader_id = 0;
    ID _next_texture_id = 0;
    ID _next_material_id = 0;
    ID _next_mesh_id = 0;

    void create_stock_shaders();
};

} // namespace glengine
