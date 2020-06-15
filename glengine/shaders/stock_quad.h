#pragma once

#include "stock_common.h"

#include <vector>

namespace {

const char *quad_vs_src =
    R"(#version 330
// vertex attributes
layout (location=0) in vec3 v_position;
layout (location=1) in vec4 v_color;
layout (location=2) in vec3 v_normal;
layout (location=3) in vec2 v_texcoord0;
// outputs
out vec2 texcoord;

void main()
{
    texcoord = v_texcoord0;
    gl_Position = vec4(v_position.x, v_position.y, 0.0, 1.0); // no perspective transformation; assume input already in view space
})";

const char *quad_fs_src =
    R"(#version 330
// uniforms
uniform sampler2D screen_texture;
uniform sampler2D g_position_texture;
uniform sampler2D g_normal_texture;
uniform sampler2D g_albedospec_texture;
uniform sampler2D ssao_texture;
// inputs
in vec2 texcoord;
// outputs
out vec4 fragment_color;

void main()
{
    vec3 col = vec3(0,0,1);
    if (texcoord.x<=0.5 && texcoord.y>0.5) {
        // top left: position
        col = texture(g_position_texture, vec2((texcoord.x)*2.0,(texcoord.y-0.5)*2.0)).rgb;
    } else if (texcoord.x>0.5 && texcoord.y>0.5) {
        // top right: normal
        col = texture(g_normal_texture, vec2((texcoord.x-0.5)*2.0,(texcoord.y-0.5)*2.0)).rgb;
    } else if (texcoord.x<=0.5 && texcoord.y<0.5) {
        // bottom left: albedo
        col = texture(g_albedospec_texture, vec2((texcoord.x)*2.0,(texcoord.y)*2.0)).rgb;
    } else if (texcoord.x>0.5 && texcoord.y<0.5) {
        // bottom right: ssao
        col = texture(ssao_texture, vec2((texcoord.x-0.5)*2.0,(texcoord.y)*2.0)).rgb;
    }
    fragment_color = vec4(col, 1.0);
})";


const char *quad_deferred_fs_src =
    R"(#version 330
// uniforms
uniform mat4 u_view;
uniform sampler2D screen_texture;
uniform sampler2D g_position_texture;
uniform sampler2D g_normal_texture;
uniform sampler2D g_albedospec_texture;
uniform sampler2D ssao_texture;
// inputs
in vec2 texcoord;
// outputs
out vec4 fragment_color;

void main()
{
    vec3 col = texture(g_albedospec_texture, texcoord).rgb;
    fragment_color = vec4(col, 1.0);
    // retrieve data from G-buffer
    vec3 frag_pos = texture(g_position_texture, texcoord).rgb;
    vec3 normal = texture(g_normal_texture, texcoord).rgb;
    vec3 albedo = texture(g_albedospec_texture, texcoord).rgb;
    float specular = texture(g_albedospec_texture, texcoord).a;
    
    // then calculate lighting as usual
    vec3 lighting = albedo * 0.01; // hard-coded ambient component
    vec3 viewDir = normalize(-frag_pos);
    // diffuse
    vec3 light_pos = vec3(u_view * vec4(100.0,100.0,100.0, 1.0));
    vec3 lightDir = normalize(light_pos - frag_pos);
    vec3 lightColor = vec3(1.0,1.0,1.0);
    // vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedo * lightColor;
    vec3 diffuse = (dot(normal, lightDir)+1.0)/2.0 * albedo * lightColor;
    lighting += diffuse;
    
    fragment_color = vec4(lighting, 1.0);
    float ssao = texture(ssao_texture, texcoord).r;
    fragment_color = vec4(lighting*ssao, 1.0);
})";

}

namespace glengine {
namespace stock_shaders {

// //// //
// quad //
// //// //

static std::vector<const char*> quad_vs_srcs = {quad_vs_src};
static std::vector<const char*> quad_fs_srcs = {quad_fs_src};

static std::vector<const char*> quad_deferred_vs_srcs = {quad_vs_src};
static std::vector<const char*> quad_deferred_fs_srcs = {quad_deferred_fs_src};

} //
} // 
