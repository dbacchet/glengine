@ctype mat4 math::Matrix4f
@ctype vec3 math::Vector3f
@ctype vec4 math::Vector4f

// ////////////////////// //
// generic implementation //
// ////////////////////// //
@block vertex_shader 

uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec4 vertex_pos;
in vec4 vertex_color;
in vec3 vertex_normal;
#ifdef TEXTURED
in vec2 vertex_texcoord;
#endif

out vec3 frag_pos;
out vec3 frag_normal;
#ifdef TEXTURED
out vec2 frag_uv;
#endif
out vec4 proj_pos;

void main() {
    frag_pos = vec3(model * vertex_pos);
    frag_normal = mat3(transpose(inverse(model))) * vertex_normal; // normal in world space - transp(inv()) is needed to take into account the scaling
#ifdef TEXTURED
    frag_uv = vertex_texcoord;
#endif
    gl_Position = projection * view * model * vertex_pos;
    proj_pos = gl_Position;
}

@end


@block fragment_shader
@include common.glsl.inc
uniform fs_params {
    vec4 color;
};

uniform sampler2D tex_diffuse;

in vec3 frag_pos;
in vec3 frag_normal;
#ifdef TEXTURED
in vec2 frag_uv;
#endif
in vec4 proj_pos;

layout(location=0) out vec4 out_frag_color;
// layout(location=1) out vec4 out_frag_normal;
// layout(location=2) out vec4 out_frag_depth;

void main() {
    // ambient
    vec3 light_color = vec3(1,1,1);
    vec3 light_pos = vec3(100,100,100);
    float ambient_strenght = 0.1;
    vec3 ambient = ambient_strenght * light_color;
  	
    // diffuse 
    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    // float diff = max(dot(norm, light_dir), 0.0); // classical approach: no light for angles >90deg
    float diff = (dot(norm, light_dir) + 1.0)/2.0; // modified (non-physically correct) approach: consider all 180deg
    vec3 diffuse = diff * light_color;
            
    vec4 tmp_color = color;
#ifdef TEXTURED
    tmp_color *= texture(tex_diffuse, frag_uv);
    if(tmp_color.a < 0.1)
        discard;
#endif
    out_frag_color = vec4(ambient + diffuse,1.0) * tmp_color;
    // out_frag_normal = vec4(frag_normal*0.5+0.5,1.0);
    // out_frag_depth = encodeDepth(proj_pos.z/proj_pos.w);
}
@end


// /////// //
// diffuse //
// /////// //

@vs vs_diffuse
@include_block vertex_shader
@end


@fs fs_diffuse
@include_block fragment_shader
@end

@program offscreen_diffuse vs_diffuse fs_diffuse

// //////////////// //
// diffuse-textured //
// //////////////// //

@vs vs_diffuse_textured
#define TEXTURED
@include_block vertex_shader
@end

@fs fs_diffuse_textured
#define TEXTURED
@include_block fragment_shader
@end

@program offscreen_diffuse_textured vs_diffuse_textured fs_diffuse_textured

