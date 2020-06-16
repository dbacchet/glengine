#pragma once

#include "stock_common.h"

#include <vector>

namespace {

// ///// //
// phong //
// ///// //

const char *phong_fs_src =
    R"(
void main() {
    // specular
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float specular_strength = 0.5;
    vec3 view_dir = normalize(-frag_pos); // the viewer is always at (0,0,0) in view-space, so view_dir is (0,0,0) - Position => -Position
    vec3 reflect_dir = reflect(-light_dir, norm);  
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    
    object_id = u_id;
    g_position = vec4(frag_pos,1.0);
    g_normal = vec4(norm,1.0);
    g_albedospec.rgb = u_color.rgb;
    g_albedospec.a = specular_strength * spec;
})";


}

namespace glengine {
namespace stock_shaders {

// ///// //
// phong //
// ///// //

static std::vector<const char*> phong_vs_srcs = {common_vs_src, default_vs_src};
static std::vector<const char*> phong_fs_srcs = {common_fs_src, phong_fs_src};

} //
} // 
