@ctype mat4 math::Matrix4f
@ctype vec4 math::Vector4f


@block vertex_shader
uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec4 vertex_pos; // since the pos is a vec3, the last coord will be initialized with 1.0 by the backend when setting the vertex format
in vec4 vertex_color;
in vec3 vertex_normal;
#ifdef TEXTURED
in vec2 vertex_texcoord;
out vec2 frag_uv;
#endif
out vec3 frag_normal;
out vec4 proj_pos;

void main() {
    gl_Position = projection * view * model * vertex_pos;
    frag_normal = mat3(transpose(inverse(model))) * vertex_normal; // normal in world space - transp(inv()) is needed to take into account the scaling
    proj_pos = gl_Position;
#ifdef TEXTURED
    frag_uv = vertex_texcoord;
#endif
}
@end

@block fragment_shader
@include common.glsl.inc
uniform fs_params {
    vec4 color;
};

uniform sampler2D tex_diffuse;

#ifdef TEXTURED
in vec2 frag_uv;
#endif
in vec3 frag_normal;
in vec4 proj_pos;

layout(location=0) out vec4 out_frag_color;
layout(location=1) out vec4 out_frag_normal;
layout(location=2) out vec4 out_frag_depth;

void main() {
    vec4 tmp_color = color;
#ifdef TEXTURED
    tmp_color *= texture(tex_diffuse, frag_uv);
    if(tmp_color.a < 0.1)
        discard;
#endif
    out_frag_color = tmp_color;
    out_frag_normal = vec4(frag_normal*0.5+0.5,1.0);
    out_frag_depth = encodeDepth(proj_pos.z/proj_pos.w);
}
@end



// //// //
// flat //
// //// //

@vs vs_flat
@include_block vertex_shader
@end

@fs fs_flat
@include_block fragment_shader
@end

@program offscreen_flat vs_flat fs_flat


// ///////////// //
// flat-textured //
// ///////////// //

@vs vs_flat_textured
#define TEXTURED
@include_block vertex_shader
@end

@fs fs_flat_textured
#define TEXTURED
@include_block fragment_shader
@end

@program offscreen_flat_textured vs_flat_textured fs_flat_textured



