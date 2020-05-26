#include "gl_resource_manager.h"

#include "math/vmath.h"

namespace {

} // namespace

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
    ShaderSrc flat_src = get_stock_shader_source(StockShader::Flat);
    ShaderSrc diffuse_src = get_stock_shader_source(StockShader::Diffuse);
    ShaderSrc diffuse_textured_src = get_stock_shader_source(StockShader::DiffuseTextured);
    ShaderSrc phong_src = get_stock_shader_source(StockShader::Phong);
    ShaderSrc vertexcolor_src = get_stock_shader_source(StockShader::VertexColor);
    ShaderSrc quad_src = get_stock_shader_source(StockShader::Quad);
    shader_flat->init(flat_src.vertex_shader, flat_src.fragment_shader);
    shader_diffuse->init(diffuse_src.vertex_shader, diffuse_src.fragment_shader);
    shader_diffuse_textured->init(diffuse_textured_src.vertex_shader, diffuse_textured_src.fragment_shader);
    shader_phong->init(phong_src.vertex_shader, phong_src.fragment_shader);
    shader_vertexcolor->init(vertexcolor_src.vertex_shader, vertexcolor_src.fragment_shader);
    shader_quad->init(quad_src.vertex_shader, quad_src.fragment_shader);
    _stock_shaders[StockShader::Flat] = shader_flat;
    _stock_shaders[StockShader::Diffuse] = shader_diffuse;
    _stock_shaders[StockShader::DiffuseTextured] = shader_diffuse_textured;
    _stock_shaders[StockShader::Phong] = shader_phong;
    _stock_shaders[StockShader::VertexColor] = shader_vertexcolor;
    _stock_shaders[StockShader::Quad] = shader_quad;
}


} // namespace glengine
