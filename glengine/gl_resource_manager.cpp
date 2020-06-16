#include "gl_resource_manager.h"
#include "gl_prefabs.h"

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

Shader *ResourceManager::create_shader() {
    ID id = _next_shader_id++;
    Shader *s = new Shader(id);
    _shaders[id] = s;
    return s;
}

Shader *ResourceManager::get_shader(ID id) {
    return _shaders[id];
}

bool ResourceManager::has_shader(ID id) const {
    return _shaders.count(id) > 0;
}

Shader *ResourceManager::get_stock_shader(StockShader type) {
    return _stock_shaders[type];
}

void ResourceManager::create_stock_shaders() {
    auto create_shader = [](const ShaderSrc &src, const char *shader_name) {
        Shader *shader = new Shader(NULL_ID, shader_name);
        shader->init(src.vertex_shader_srcs, src.fragment_shader_srcs);
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

Texture *ResourceManager::create_texture() {
    ID id = _next_texture_id++;
    Texture *t = new Texture(id);
    _textures[id] = t;
    return t;
}

Texture *ResourceManager::get_texture(ID id) {
    return _textures[id];
}

bool ResourceManager::has_texture(ID id) const {
    return _textures.count(id) > 0;
}

Texture *ResourceManager::create_texture_from_file(const char *filename) {
    Texture *t = create_texture();
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

Texture *ResourceManager::create_texture_from_data(uint32_t width, uint32_t height, uint8_t channels, const uint8_t *data) {
    Texture *t = create_texture();
    if (t && data) {
        t->init(width, height, data, GL_SRGB_ALPHA, channels < 4 ? GL_RGB : GL_RGBA);
    } else {
        printf("Failed to generate/load texture\n");
    }
    return t;
}

Mesh *ResourceManager::create_mesh() {
    ID id = _next_mesh_id++;
    Mesh *t = new Mesh(id);
    _meshes[id] = t;
    return t;
}

Mesh *ResourceManager::get_mesh(ID id) {
    return _meshes[id];
}

bool ResourceManager::has_mesh(ID id) const {
    return _meshes.count(id) > 0;
}

Mesh *ResourceManager::create_axis_mesh() {
    Mesh *m = create_mesh();
    MeshData md = create_axis_data();
    m->init(md.vertices, md.indices, GL_LINES);
    return m;
}

Mesh *ResourceManager::create_quad_mesh() {
    Mesh *m = create_mesh();
    MeshData md = create_quad_data();
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *ResourceManager::create_box_mesh(const math::Vector3f &size) {
    Mesh *m = create_mesh();
    MeshData md = create_box_data(size);
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *ResourceManager::create_sphere_mesh(float radius, uint32_t subdiv) {
    Mesh *m = create_mesh();
    MeshData md = create_sphere_data(radius, subdiv);
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *ResourceManager::create_grid_mesh(float len, float step) {
    Mesh *m = create_mesh();
    MeshData md = create_grid_data(len, step);
    m->init(md.vertices, md.indices, GL_LINES);
    return m;
}

} // namespace glengine
