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
uniform uint u_id;
in vec4 color;
layout (location = 0) out vec4 fragment_color;
layout (location = 1) out uint object_id;  

void main() {
    fragment_color = color;
    object_id = u_id;
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
uniform uint u_id;
uniform vec4 u_color;
layout (location = 0) out vec4 fragment_color;
layout (location = 1) out uint object_id;  

void main() {
    fragment_color = u_color;
    object_id = u_id;
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
uniform uint u_id;
uniform vec4 u_color;
// inputs
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos;
// in vec4 vcolor;
// output
layout (location = 0) out vec4 fragment_color;
layout (location = 1) out uint object_id;  
layout (location = 2) out vec4 g_position;  
layout (location = 3) out vec4 g_normal;  
layout (location = 4) out vec4 g_albedospec;  

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

// //////////////// //
// diffuse textured //
// //////////////// //

const char *diffuse_textured_vs_src =
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
// out vec4 vcolor;

void main() {
    frag_pos = vec3(u_view * u_model * vec4(v_position,1.0));       // fragment position in view-space
    normal = mat3(transpose(inverse(u_view * u_model))) * v_normal; // normal in view space - transp(inv()) is needed to take into account the scaling
    light_pos = vec3(u_view * vec4(u_light0_pos, 1.0));             // world-space light position to view-space light position
    tex_coord = v_texcoord0;
    // vcolor   = v_color;
    gl_Position = u_projection * u_view * u_model * vec4(v_position,1.0);
})";

const char *diffuse_textured_fs_src =
    R"(#version 330
uniform uint u_id;
uniform vec4 u_color;
uniform sampler2D texture_diffuse;
// inputs
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos;
in vec2 tex_coord;
// in vec4 vcolor;
// output
layout (location = 0) out vec4 fragment_color;
layout (location = 1) out uint object_id;  
layout (location = 2) out vec4 g_position;  
layout (location = 3) out vec4 g_normal;  
layout (location = 4) out vec4 g_albedospec;  

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

    // vec4 color = vec4(tex_coord.x,0.0,tex_coord.y,1.0);//texture(texture_diffuse, tex_coord);
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

// ///// //
// phong //
// ///// //

const char *phong_vs_src = diffuse_vs_src; // the vertex shader is the same for diffuse and phong

const char *phong_fs_src =
    R"(#version 330
uniform uint u_id;
uniform vec4 u_color;
// inputs
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos;
// in vec4 vcolor;
// output
layout (location = 0) out vec4 fragment_color;
layout (location = 1) out uint object_id;  

void main() {
    // params
    float ambient_strength = 0.01;
    float specular_strength = 0.5;
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
    vec3 view_dir = normalize(-frag_pos); // the viewer is always at (0,0,0) in view-space, so view_dir is (0,0,0) - Position => -Position
    vec3 reflect_dir = reflect(-light_dir, norm);  
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color; 
    
    vec3 result = (ambient + diffuse + specular) * u_color.xyz; //vcolor.xyz;
    fragment_color = vec4(result, u_color.a);
    object_id = u_id;
})";

// //// //
// quad //
// //// //

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
    gl_Position = vec4(v_position.x, v_position.y, 0.0, 1.0); 
})";

const char *quad_fs_src =
    R"(#version 330
// uniforms
uniform sampler2D screen_texture;
// inputs
in vec2 texcoord;
// outputs
out vec4 fragment_color;

void main()
{
    vec3 col = texture(screen_texture, texcoord).rgb;
    fragment_color = vec4(col, 1.0);
})";

const char *quad_deferred_vs_src =
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
    gl_Position = vec4(v_position.x, v_position.y, 0.0, 1.0); 
})";

const char *quad_deferred_fs_src =
    R"(#version 330
// uniforms
uniform sampler2D screen_texture;
uniform sampler2D g_position_texture;
uniform sampler2D g_normal_texture;
uniform sampler2D g_albedospec_texture;
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
    vec3 lighting = albedo * 0.1; // hard-coded ambient component
    vec3 viewDir = normalize(-frag_pos);
        // diffuse
        vec3 lightDir = normalize(vec3(0,0,0) - frag_pos);
        vec3 lightColor = vec3(1.0,1.0,1.0);
        vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedo * lightColor;
        lighting += diffuse;
    
    fragment_color = vec4(lighting, 1.0);
})";


} // namespace

namespace glengine {

ShaderSrc get_stock_shader_source(StockShader type) {
    switch (type) {
    case StockShader::Flat:
        return {flat_vs_src, flat_fs_src};
    case StockShader::Diffuse:
        return {diffuse_vs_src, diffuse_fs_src};
    case StockShader::DiffuseTextured:
        return {diffuse_textured_vs_src, diffuse_textured_fs_src};
    case StockShader::Phong:
        return {phong_vs_src, phong_fs_src};
    case StockShader::Quad:
        return {quad_vs_src, quad_fs_src};
    case StockShader::QuadDeferred:
        return {quad_deferred_vs_src, quad_deferred_fs_src};
    default: // use vertexcolor by default
        return {vertexcolor_vs_src, vertexcolor_fs_src};
    }
}

} // namespace glengine
