#pragma once

namespace glengine {
namespace stock_shaders {

// ////// //
// common //
// ////// //

const char *common_vs_src =
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
out vec2 tex_coord;
out vec4 vcolor;
)";

const char *default_vs_src =
    R"(
void main() {
    frag_pos = vec3(u_view * u_model * vec4(v_position,1.0));       // fragment position in view-space
    normal = mat3(transpose(inverse(u_view * u_model))) * v_normal; // normal in view space - transp(inv()) is needed to take into account the scaling
    light_pos = vec3(u_view * vec4(u_light0_pos, 1.0));             // world-space light position to view-space light position
    tex_coord = v_texcoord0;
    vcolor   = v_color;
    gl_Position = u_projection * u_view * u_model * vec4(v_position,1.0);
})";


const char *common_fs_src =
    R"(#version 330
uniform uint u_id;
uniform vec4 u_color;
// inputs
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos;
in vec2 tex_coord;
in vec4 vcolor;
// output
layout (location = 0) out vec4 fragment_color;
layout (location = 1) out uint object_id;  
layout (location = 2) out vec4 g_position;  
layout (location = 3) out vec4 g_normal;  
layout (location = 4) out vec4 g_albedospec;  
)";

} //
} // 
