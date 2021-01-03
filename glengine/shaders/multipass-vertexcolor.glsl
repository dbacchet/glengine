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

out vec4 color;

void main() {
    gl_Position = projection * view * model * vertex_pos;
    color = vertex_col;
}
@end

@fs fs_vertexcolor
uniform fs_params {
    uint object_id;
};

in vec4 color;

layout(location=0) out vec4 frag_color;

void main() {
    frag_color = color;
}
@end

@program offscreen_vertexcolor vs_vertexcolor fs_vertexcolor



