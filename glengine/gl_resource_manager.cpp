#include "gl_resource_manager.h"
#include "gl_prefabs.h"
#include "gl_material.h"
#include "gl_mesh.h"
#include "gl_texture.h"

#include "math/vmath.h"
#include "stb/stb_image.h"

namespace {} // namespace

namespace glengine {

ResourceManager::~ResourceManager() {}

bool ResourceManager::init() {
    create_stock_shaders();
    return true;
}

bool ResourceManager::terminate() {
    // deallocate all resources
    // shaders
    for (auto it : _stock_shaders) {
        delete it.second;
    }
    for (auto it : _shaders) {
        delete it.second;
    }
    _shaders.clear();
    return true;
}

Shader *ResourceManager::create_shader(const char *name) {
    ID id = _next_shader_id++;
    Shader *s = new Shader(id, name);
    _shaders[id] = s;
    return s;
}

Shader *ResourceManager::get_shader(ID id) {
    if (!has_shader(id)) {
        return nullptr;
    }
    return _shaders[id];
}

std::set<Shader*> ResourceManager::get_shader(const char *name) {
    if (!has_shader(name)) {
        return {};
    }
    return _shader_lookup[name];
}

bool ResourceManager::has_shader(ID id) const {
    return _shaders.count(id) > 0;
}

bool ResourceManager::has_shader(const char *name) const {
    return _shader_lookup.count(name) > 0;
}

Shader *ResourceManager::get_stock_shader(StockShader type) {
    return _stock_shaders[type];
}

void ResourceManager::create_stock_shaders() {
    auto create_shader = [this](const ShaderSrc &src, const char *shader_name) {
        Shader *shader = this->create_shader(shader_name);
        if (shader) {
            shader->init(src.vertex_shader_srcs, src.fragment_shader_srcs);
        }
        return shader;
    };
    _stock_shaders[StockShader::Flat]            = create_shader(get_stock_shader_source(StockShader::Flat), "Flat");
    _stock_shaders[StockShader::FlatTextured]    = create_shader(get_stock_shader_source(StockShader::FlatTextured), "FlatTextured");
    _stock_shaders[StockShader::Diffuse]         = create_shader(get_stock_shader_source(StockShader::Diffuse), "Diffuse");
    _stock_shaders[StockShader::DiffuseTextured] = create_shader(get_stock_shader_source(StockShader::DiffuseTextured), "DiffuseTextured");
    _stock_shaders[StockShader::Phong]           = create_shader(get_stock_shader_source(StockShader::Phong), "Phong");
    _stock_shaders[StockShader::VertexColor]     = create_shader(get_stock_shader_source(StockShader::VertexColor), "VertexColor");
}

Texture *ResourceManager::create_texture(const char *name) {
    ID id = _next_texture_id++;
    Texture *t = new Texture(id, name);
    if (t) {
        _textures[id] = t;
        _texture_lookup[name].insert(t);
    }
    return t;
}

Texture *ResourceManager::get_texture(ID id) {
    if (!has_texture(id)) {
        return nullptr;
    }
    return _textures[id];
}

std::set<Texture*> ResourceManager::get_texture(const char *name) {
    if (!has_texture(name)) {
        return {};
    }
    return _texture_lookup[name];
}

bool ResourceManager::has_texture(ID id) const {
    return _textures.count(id) > 0;
}

bool ResourceManager::has_texture(const char *name) const {
    return _texture_lookup.count(name) > 0;
}

Texture *ResourceManager::create_texture_from_file(const char *filename) {
    Texture *t = create_texture(filename);
    // load and generate the texture
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (t && data) {
        t->init(width, height, data, GL_RGBA, nrChannels < 4 ? GL_RGB : GL_RGBA);
    } else {
        printf("Failed to generate/load texture\n");
    }
    stbi_image_free(data);
    return t;
}

Texture *ResourceManager::create_texture_from_data(const char *name, uint32_t width, uint32_t height, uint8_t channels, const uint8_t *data) {
    Texture *t = create_texture(name);
    if (t && data) {
        t->init(width, height, data, GL_RGBA, channels < 4 ? GL_RGB : GL_RGBA);
    } else {
        printf("Failed to generate/load texture\n");
    }
    return t;
}

Material *ResourceManager::create_material(const char *name, Shader *shader) {
    ID id = _next_material_id++;
    Material *m = new Material(id, name, shader);
    if (m) {
        _materials[id] = m;
        _material_lookup[name].insert(m);
    }
    return m;
}

Material *ResourceManager::create_material(const char *name, StockShader type) {
    return create_material(name, get_stock_shader(type));
}

Material *ResourceManager::get_material(ID id) {
    if (!has_material(id)) {
        return nullptr;
    }
    return _materials[id];
}

std::set<Material*> ResourceManager::get_material(const char *name) {
    if (!has_material(name)) {
        return {};
    }
    return _material_lookup[name];
}

bool ResourceManager::has_material(ID id) const {
    return _materials.count(id) > 0;
}

bool ResourceManager::has_material(const char *name) const {
    return _material_lookup.count(name) > 0;
}


Mesh *ResourceManager::create_mesh(const char *name) {
    ID id = _next_mesh_id++;
    Mesh *m = new Mesh(id, name);
    if (m) {
        _meshes[id] = m;
        _mesh_lookup[name].insert(m);
    }
    return m;
}

Mesh *ResourceManager::get_mesh(ID id) {
    if (!has_mesh(id)) {
        return nullptr;
    }
    return _meshes[id];
}

std::set<Mesh*> ResourceManager::get_mesh(const char *name) {
    if (!has_mesh(name)) {
        return {};
    }
    return _mesh_lookup[name];
}

bool ResourceManager::has_mesh(ID id) const {
    return _meshes.count(id) > 0;
}

bool ResourceManager::has_mesh(const char *name) const {
    return _mesh_lookup.count(name) > 0;
}

Mesh *ResourceManager::create_axis_mesh(const char *name) {
    Mesh *m = create_mesh(name);
    MeshData md = create_axis_data();
    m->init(md.vertices, md.indices, GL_LINES);
    return m;
}

Mesh *ResourceManager::create_quad_mesh(const char *name) {
    Mesh *m = create_mesh(name);
    MeshData md = create_quad_data();
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *ResourceManager::create_box_mesh(const char *name, const math::Vector3f &size) {
    Mesh *m = create_mesh(name);
    MeshData md = create_box_data(size);
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *ResourceManager::create_sphere_mesh(const char *name, float radius, uint32_t subdiv) {
    Mesh *m = create_mesh(name);
    MeshData md = create_sphere_data(radius, subdiv);
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *ResourceManager::create_grid_mesh(const char *name, float len, float step) {
    Mesh *m = create_mesh(name);
    MeshData md = create_grid_data(len, step);
    m->init(md.vertices, md.indices, GL_LINES);
    return m;
}

} // namespace glengine
