@ctype mat4 math::Matrix4f
@ctype vec4 math::Vector4f

@vs vs_flat
uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec4 vertex_pos;

void main() {
    gl_Position = projection * view * model * vertex_pos;
}
@end

@fs fs_flat
uniform fs_params {
    vec4 color;
};

// uniform uint object_id;

layout(location=0) out vec4 frag_color;

void main() {
    frag_color = color;
}
@end

@program offscreen_flat vs_flat fs_flat



