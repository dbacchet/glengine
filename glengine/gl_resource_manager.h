#pragma once

#include "sokol_gfx.h"

#include <string>
#include <set>
#include <array>
#include <unordered_map>

namespace glengine {

class Material;
class Mesh;

/// class used to manage resources (materials, shaders, pipelines etc.)
class ResourceManager {
public:
    enum DefaultImage {
        White,  ///< rgba image with all white pixels, and alpha 0xFF
        Black,  ///< rgba image with all black pixels, and alpha 0xFF
        Normal, ///< rgba image corresponsing to z (0,0,1) normal vector
        DefaultImageNum,
    };

    ~ResourceManager();

    void init();
    void terminate();

    /// image creation/retrieval
    sg_image get_or_create_image(const sg_image_desc &desc);
    sg_image get_or_create_image(const char *filename);
    sg_image get_or_create_image(const uint8_t *data, int32_t len);
    /// default images
    sg_image default_image(DefaultImage type);
    /// shader creation/retrieval
    sg_shader get_or_create_shader(const sg_shader_desc &desc);
    /// pipeline creation/retrieval
    sg_pipeline get_or_create_pipeline(const sg_pipeline_desc &desc);

    void register_material(Material *mtl);
    void register_mesh(Mesh *msh);

    std::array<sg_image, DefaultImageNum> _default_images;
    std::unordered_map<uint64_t, sg_image> _images; 
    std::unordered_map<uint64_t, sg_shader> _shaders; 
    std::unordered_map<uint64_t, sg_pipeline> _pipelines; 
    std::set<Material*> _materials;
    std::set<Mesh*> _meshes;

};

} //
