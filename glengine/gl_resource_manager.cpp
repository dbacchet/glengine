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
    Shader *shader_flat = new Shader(NULL_ID, "Flat");
    Shader *shader_diffuse = new Shader(NULL_ID, "Diffuse");
    Shader *shader_diffuse_textured = new Shader(NULL_ID, "DiffuseTextured");
    Shader *shader_phong = new Shader(NULL_ID, "Phong");
    Shader *shader_vertexcolor = new Shader(NULL_ID, "VertexColor");
    Shader *shader_quad = new Shader(NULL_ID, "ScreenQuad");
    Shader *shader_quad_deferred = new Shader(NULL_ID, "ScreenQuadDeferred");
    ShaderSrc flat_src = get_stock_shader_source(StockShader::Flat);
    ShaderSrc diffuse_src = get_stock_shader_source(StockShader::Diffuse);
    ShaderSrc diffuse_textured_src = get_stock_shader_source(StockShader::DiffuseTextured);
    ShaderSrc phong_src = get_stock_shader_source(StockShader::Phong);
    ShaderSrc vertexcolor_src = get_stock_shader_source(StockShader::VertexColor);
    ShaderSrc quad_src = get_stock_shader_source(StockShader::Quad);
    ShaderSrc quad_deferred_src = get_stock_shader_source(StockShader::QuadDeferred);
    shader_flat->init(flat_src.vertex_shader, flat_src.fragment_shader);
    shader_diffuse->init(diffuse_src.vertex_shader, diffuse_src.fragment_shader);
    shader_diffuse_textured->init(diffuse_textured_src.vertex_shader, diffuse_textured_src.fragment_shader);
    shader_phong->init(phong_src.vertex_shader, phong_src.fragment_shader);
    shader_vertexcolor->init(vertexcolor_src.vertex_shader, vertexcolor_src.fragment_shader);
    shader_quad->init(quad_src.vertex_shader, quad_src.fragment_shader);
    shader_quad_deferred->init(quad_deferred_src.vertex_shader, quad_deferred_src.fragment_shader);
    _stock_shaders[StockShader::Flat] = shader_flat;
    _stock_shaders[StockShader::Diffuse] = shader_diffuse;
    _stock_shaders[StockShader::DiffuseTextured] = shader_diffuse_textured;
    _stock_shaders[StockShader::Phong] = shader_phong;
    _stock_shaders[StockShader::VertexColor] = shader_vertexcolor;
    _stock_shaders[StockShader::Quad] = shader_quad;
    _stock_shaders[StockShader::QuadDeferred] = shader_quad_deferred;
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
