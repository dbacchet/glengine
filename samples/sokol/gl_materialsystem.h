#pragma once

#include "sokol_gfx.h"

#include <unordered_map>

namespace glengine {

class Material;

/// class used to manage materials, shaders and pipelines
class MaterialSystem {
public:

    ~MaterialSystem();

    static MaterialSystem& get();

    sg_shader get_or_create_shader(const sg_shader_desc &desc);
    sg_pipeline get_or_create_pipeline(const sg_pipeline_desc &desc);

    std::unordered_map<uint64_t, sg_shader> _shaders; 
    std::unordered_map<uint64_t, sg_pipeline> _pipelines; 
    // std::unordered_map<uint32_t, Material*> _materials; 

};

} //
