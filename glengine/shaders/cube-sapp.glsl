@ctype mat4 math::Matrix4f

@vs vs
uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec4 position;
in vec4 color0;

out vec4 color;

void main() {
    gl_Position = projection * view * model * position;
    color = color0;
}
@end

@fs fs
in vec4 color;
out vec4 frag_color;

void main() {
    frag_color = color;
}
@end

@program cube vs fs
