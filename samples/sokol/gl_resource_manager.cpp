#include "gl_resource_manager.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_material.h"
#include "gl_mesh.h"

namespace glengine {

ResourceManager::~ResourceManager() {
    // nothing to be done here
}

void ResourceManager::terminate() {
    // cleanup meshes
    log_info("ResourceManager: cleanup meshes");
    for (auto &mesh : _meshes) {
        log_debug("Destroying mesh %p", &mesh);
        delete mesh;
    }
    // cleanup materials
    log_info("ResourceManager: cleanup materials");
    for (auto &mtl : _materials) {
        log_debug("Destroying material %p", &mtl);
        delete mtl;
    }
    _materials.clear();
    // cleanup pipeline resources
    log_info("ResourceManager: cleanup pipelines");
    for (auto it : _pipelines) {
        log_debug("Destroying pipeline %u", it.second.id);
        sg_destroy_pipeline(it.second);
    }
    _pipelines.clear();
    // cleanup shader resources
    log_info("ResourceManager: cleanup shaders");
    for (auto it : _shaders) {
        log_debug("Destroying shader %u", it.second.id);
        sg_destroy_shader(it.second);
    }
    _shaders.clear();
}

sg_shader ResourceManager::get_or_create_shader(const sg_shader_desc &desc) {
    uint64_t shader_hash = murmur_hash2_64(&desc, sizeof(desc), 12345678);
    if (_shaders.count(shader_hash) > 0) {
        return _shaders[shader_hash];
    }
    // create the shader and add it to the cache
    log_info("Creating shader %s", desc.label);
    sg_shader shd = sg_make_shader(desc);
    log_info("Created shader %u", shd.id);
    _shaders[shader_hash] = shd;
    return shd;
}

sg_pipeline ResourceManager::get_or_create_pipeline(const sg_pipeline_desc &desc) {
    uint64_t pipeline_hash = murmur_hash2_64(&desc, sizeof(desc), 12345678);
    if (_pipelines.count(pipeline_hash) > 0) {
        return _pipelines[pipeline_hash];
    }
    // create the pipeline and add it to the cache
    log_info("Creating pipeline [%s]", desc.label);
    sg_pipeline pip = sg_make_pipeline(desc);
    log_info("Created pipeline %u", pip.id);
    _pipelines[pipeline_hash] = pip;
    return pip;
}

void ResourceManager::register_material(Material *mtl) {
    _materials.insert(mtl);
}

void ResourceManager::register_mesh(Mesh *msh) {
    _meshes.insert(msh);
}

} // namespace glengine
