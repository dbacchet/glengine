#pragma once

#include "sokol_gfx.h"

#include <string>
#include <set>
#include <unordered_map>

namespace glengine {

class Material;
class Mesh;

/// class used to manage resources (materials, shaders, pipelines etc.)
class ResourceManager {
public:

    ~ResourceManager();

    void init() {}
    void terminate();

    /// image creation/retrieval
    sg_image get_or_create_image(const sg_image_desc &desc);
    sg_image get_or_create_image(const char *filename);
    sg_image get_or_create_image(const uint8_t *data, int32_t len);
    /// shader creation/retrieval
    sg_shader get_or_create_shader(const sg_shader_desc &desc);
    /// pipeline creation/retrieval
    sg_pipeline get_or_create_pipeline(const sg_pipeline_desc &desc);

    void register_material(Material *mtl);
    void register_mesh(Mesh *msh);

    std::unordered_map<uint64_t, sg_image> _images; 
    std::unordered_map<uint64_t, sg_shader> _shaders; 
    std::unordered_map<uint64_t, sg_pipeline> _pipelines; 
    std::set<Material*> _materials;
    std::set<Mesh*> _meshes;

};

} //
