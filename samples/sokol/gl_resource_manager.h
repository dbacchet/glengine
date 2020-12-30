#pragma once

#include "sokol_gfx.h"

#include <unordered_map>

namespace glengine {

class Material;

/// class used to manage resources (materials, shaders, pipelines etc.)
class ResourceManager {
public:

    ~ResourceManager();

    void init() {}
    void terminate();

    /// shader creation/retrieval
    sg_shader get_or_create_shader(const sg_shader_desc &desc);
    /// pipeline creation/retrieval
    sg_pipeline get_or_create_pipeline(const sg_pipeline_desc &desc);

    std::unordered_map<uint64_t, sg_shader> _shaders; 
    std::unordered_map<uint64_t, sg_pipeline> _pipelines; 

};

} //
