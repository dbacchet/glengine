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
    ID id = murmur_hash2_32(name);
    if (_shaders.count(id)>0) {
        printf("*** Error Creating Shader ***\nShader with name '%s' already exists\n", name);
        return nullptr;
    }
    Shader *s = new Shader(name);
    _shaders[id] = s;
    return s;
}

Shader *ResourceManager::get_shader(const char *name) {
    return _shaders[murmur_hash2_32(name)];
}

bool ResourceManager::has_shader(const char *name) const {
    return _shaders.count(murmur_hash2_32(name)) > 0;
}

Shader *ResourceManager::get_stock_shader(StockShader type) {
    return _stock_shaders[type];
}

void ResourceManager::create_stock_shaders() {
    auto create_shader = [](const ShaderSrc &src, const char *shader_name) {
        Shader *shader = new Shader(shader_name);
        if (shader) {
            shader->init(src.vertex_shader_srcs, src.fragment_shader_srcs);
        }
        return shader;
    };
    _stock_shaders[StockShader::Flat]            = create_shader(get_stock_shader_source(StockShader::Flat), "Flat");
    _stock_shaders[StockShader::Diffuse]         = create_shader(get_stock_shader_source(StockShader::Diffuse), "Diffuse");
    _stock_shaders[StockShader::DiffuseTextured] = create_shader(get_stock_shader_source(StockShader::DiffuseTextured), "DiffuseTextured");
    _stock_shaders[StockShader::Phong]           = create_shader(get_stock_shader_source(StockShader::Phong), "Phong");
    _stock_shaders[StockShader::VertexColor]     = create_shader(get_stock_shader_source(StockShader::VertexColor), "VertexColor");
    _stock_shaders[StockShader::Ssao]            = create_shader(get_stock_shader_source(StockShader::Ssao), "Ssao");
    _stock_shaders[StockShader::QuadDebug]       = create_shader(get_stock_shader_source(StockShader::QuadDebug), "Quad");
    _stock_shaders[StockShader::QuadDeferred]    = create_shader(get_stock_shader_source(StockShader::QuadDeferred), "QuadDeferred");
}

Texture *ResourceManager::create_texture(const char *name) {
    ID id = murmur_hash2_32(name);
    if (_textures.count(id)>0) {
        printf("*** Error Creating Texture ***\nTexture with name '%s' already exists\n", name);
        return nullptr;
    }
    Texture *t = new Texture(name);
    _textures[id] = t;
    return t;
}

Texture *ResourceManager::get_texture(const char *name) {
    return _textures[murmur_hash2_32(name)];
}

bool ResourceManager::has_texture(const char *name) const {
    return _textures.count(murmur_hash2_32(name)) > 0;
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
        t->init(width, height, data, GL_SRGB_ALPHA, channels < 4 ? GL_RGB : GL_RGBA);
    } else {
        printf("Failed to generate/load texture\n");
    }
    return t;
}

Material *ResourceManager::create_material(const char *name) {
    ID id = murmur_hash2_32(name);
    if (_materials.count(id)>0) {
        printf("*** Error Creating Material ***\nMaterial with name '%s' already exists\n", name);
        return nullptr;
    }
    Material *t = new Material(name);
    _materials[id] = t;
    return t;
}

Material *ResourceManager::get_material(const char *name) {
    return _materials[murmur_hash2_32(name)];
}

bool ResourceManager::has_material(const char *name) const {
    return _materials.count(murmur_hash2_32(name)) > 0;
}


Mesh *ResourceManager::create_mesh(const char *name) {
    ID id = murmur_hash2_32(name);
    if (_meshes.count(id)>0) {
        printf("*** Error Creating Mesh ***\nMesh with name '%s' already exists\n", name);
        return nullptr;
    }
    Mesh *t = new Mesh(name);
    _meshes[id] = t;
    return t;
}

Mesh *ResourceManager::get_mesh(const char *name) {
    return _meshes[murmur_hash2_32(name)];
}

bool ResourceManager::has_mesh(const char *name) const {
    return _meshes.count(murmur_hash2_32(name)) > 0;
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
