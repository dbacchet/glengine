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
    float ambient_strength = 0.01;
    vec3 light_color = vec3(1.0,1.0,1.0);
    // ambient
    vec3 ambient = ambient_strength * light_color;    
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    // float diff = max(dot(norm, light_dir), 0.0); // classical approach: no light for angles >90deg
    float diff = (dot(norm, light_dir) + 1.0)/2.0; // modified (non-physically correct) approach: consider all 180deg
    vec3 diffuse = diff * light_color;
    // specular
    float specular_strength = 0.5;
    vec3 view_dir = normalize(-frag_pos); // the viewer is always at (0,0,0) in view-space, so view_dir is (0,0,0) - Position => -Position
    vec3 reflect_dir = reflect(-light_dir, norm);  
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color; 
    
    vec3 result = (ambient + diffuse + specular) * u_color.xyz; //vcolor.xyz;
    fragment_color = vec4(result, 1.0);
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
