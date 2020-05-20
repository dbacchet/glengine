#include "gl_stock_shaders.h"

namespace {

// /////////// //
// vertexcolor //
// /////////// //

const char *vertexcolor_vs_src =
    R"(#version 330
// uniforms
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
// vertex attributes
layout (location=0) in vec3 v_position;
layout (location=1) in vec4 v_color;
layout (location=2) in vec3 v_normal;
layout (location=3) in vec2 v_texcoord0;
// outputs
out vec4 color;

void main() {
    gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);
    color = v_color;
})";

const char *vertexcolor_fs_src =
    R"(#version 330
in vec4 color;
out vec4 fragment_color;

void main() {
    fragment_color = color;
})";

// //// //
// flat //
// //// //

const char *flat_vs_src =
    R"(#version 330
// uniforms
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
// vertex attributes
layout (location=0) in vec3 v_position;
layout (location=1) in vec4 v_color;
layout (location=2) in vec3 v_normal;
layout (location=3) in vec2 v_texcoord0;

void main() {
    gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);
})";

const char *flat_fs_src =
    R"(#version 330
uniform vec4 u_color;
out vec4 fragment_color;

void main() {
    fragment_color = u_color;
})";

// /////// //
// diffuse //
// /////// //

const char *diffuse_vs_src =
    R"(#version 330
// uniforms
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_light0_pos;
// vertex attributes
layout (location=0) in vec3 v_position;
layout (location=1) in vec4 v_color;
layout (location=2) in vec3 v_normal;
layout (location=3) in vec2 v_texcoord0;
// outputs
out vec3 frag_pos;
out vec3 normal;
out vec3 light_pos;
// out vec4 vcolor;

void main() {
    frag_pos = vec3(u_view * u_model * vec4(v_position,1.0));       // fragment position in view-space
    normal = mat3(transpose(inverse(u_view * u_model))) * v_normal; // normal in view space - transp(inv()) is needed to take into account the scaling
    light_pos = vec3(u_view * vec4(u_light0_pos, 1.0));             // world-space light position to view-space light position
    // vcolor   = v_color;
    gl_Position = u_projection * u_view * u_model * vec4(v_position,1.0);
})";

const char *diffuse_fs_src =
    R"(#version 330
// uniform float u_shininess;
uniform vec4 u_color;
// inputs
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos;
// in vec4 vcolor;
// output
out vec4 fragment_color;

void main() {
    // params
    float ambient_strength = 0.10;
    vec3 light_color = vec3(1.0,1.0,1.0);
    // ambient
    vec3 ambient = ambient_strength * light_color;    
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;
    
    vec3 result = (ambient + diffuse) * u_color.xyz;//vcolor.xyz;
    fragment_color = vec4(result, 1.0);
})";

// ///// //
// phong //
// ///// //

const char *phong_vs_src = diffuse_vs_src; // the vertex shader is the same for diffuse and phong

const char *phong_fs_src =
    R"(#version 330
// uniform float u_shininess;
uniform vec4 u_color;
// inputs
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos;
// in vec4 vcolor;
// output
out vec4 fragment_color;

void main() {
    // params
    float ambient_strength = 0.10;
    float specular_strength = 0.5;
    vec3 light_color = vec3(1.0,1.0,1.0);
    // ambient
    vec3 ambient = ambient_strength * light_color;    
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;
    // specular
    vec3 view_dir = normalize(-frag_pos); // the viewer is always at (0,0,0) in view-space, so view_dir is (0,0,0) - Position => -Position
    vec3 reflect_dir = reflect(-light_dir, norm);  
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color; 
    
    vec3 result = (ambient + diffuse + specular) * u_color.xyz; //vcolor.xyz;
    fragment_color = vec4(result, 1.0);
})";

} // namespace

namespace glengine {

ShaderSrc get_stock_shader_source(StockShader type) {
    switch (type) {
    case StockShader::Flat:
        return {flat_vs_src, flat_fs_src};
    case StockShader::Diffuse:
        return {diffuse_vs_src, diffuse_fs_src};
    case StockShader::Phong:
        return {phong_vs_src, phong_fs_src};
    default: // use vertexcolor by default
        return {vertexcolor_vs_src, vertexcolor_fs_src};
    }
}

} // namespace glengine
