@ctype mat4 math::Matrix4f
@ctype vec2 math::Matrix2f

@vs vs_vertexcolor
uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec4 vertex_pos;
in vec4 vertex_col;
in vec3 vertex_normal;

out vec4 color;
out vec3 frag_normal;
out vec4 proj_pos;

void main() {
    gl_Position = projection * view * model * vertex_pos;
    frag_normal = mat3(transpose(inverse(model))) * vertex_normal; // normal in world space - transp(inv()) is needed to take into account the scaling
    proj_pos = gl_Position;
    color = vertex_col;
}
@end

@fs fs_vertexcolor
@include common.glsl.inc
uniform fs_params {
    uint object_id;
};

in vec4 color;
in vec3 frag_normal;
in vec4 proj_pos;

layout(location=0) out vec4 out_frag_color;
// layout(location=1) out vec4 out_frag_normal;
// layout(location=2) out vec4 out_frag_depth;

void main() {
    out_frag_color = color;
    // out_frag_normal = vec4(frag_normal*0.5+0.5,1.0);
    // out_frag_depth = encodeDepth(proj_pos.z/proj_pos.w);
}
@end

@program offscreen_vertexcolor vs_vertexcolor fs_vertexcolor
