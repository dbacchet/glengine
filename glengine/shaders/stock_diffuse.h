#pragma once

#include "stock_common.h"

#include <vector>

namespace {

const char *diffuse_fs_src =
    R"(
void main() {
    // params
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
    
    vec3 result = (ambient + diffuse) * u_color.xyz;//vcolor.xyz;
    // output
    fragment_color = vec4(result, u_color.a);
    object_id = u_id;
    g_position = vec4(frag_pos,1.0);
    g_normal = vec4(norm,1.0);
    g_albedospec.rgb = u_color.rgb;
    g_albedospec.a = u_color.a;
})";


const char *diffuse_textured_fs_src =
    R"(
// additional uniforms not defined in the common part
uniform sampler2D texture_diffuse;

void main() {
    // params
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

    vec4 color = texture(texture_diffuse, tex_coord);
    if(color.a < 0.1)
        discard;
    vec3 result = (ambient + diffuse) * color.xyz;//vcolor.xyz;
    // output
    fragment_color = vec4(result, color.a);
    object_id = u_id;
    g_position = vec4(frag_pos,1.0);
    g_normal = vec4(norm,1.0);
    g_albedospec.rgb = color.rgb;
    g_albedospec.a = color.a;
})";


}

namespace glengine {
namespace stock_shaders {

// /////// //
// diffuse //
// /////// //

static std::vector<const char*> diffuse_vs_srcs = {common_vs_src, default_vs_src};
static std::vector<const char*> diffuse_fs_srcs = {common_fs_src, diffuse_fs_src};

// //////////////// //
// diffuse textured //
// //////////////// //

static std::vector<const char*> diffuse_textured_vs_srcs = {common_vs_src, default_vs_src};
static std::vector<const char*> diffuse_textured_fs_srcs = {common_fs_src, diffuse_textured_fs_src};

} //
} // 
