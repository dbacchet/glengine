@ctype mat4 math::Matrix4f
@ctype vec2 math::Matrix2f

@vs vs_vertexcolor
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

@fs fs_vertexcolor
in vec4 color;

layout(location=0) out vec4 frag_color;

void main() {
    frag_color = color;
}
@end

@program offscreen_vertexcolor vs_vertexcolor fs_vertexcolor


// shaders for rendering a fullscreen-quad in default pass
@vs vs_fsq
/* @glsl_options flip_vert_y */

in vec2 pos;

out vec2 uv0;

void main() {
    gl_Position = vec4(pos*2.0-1.0, 0.5, 1.0);
    uv0 = pos;
}
@end

@fs fs_fsq
uniform sampler2D tex0;

in vec2 uv0;

out vec4 frag_color;

void main() {
    vec3 c0 = texture(tex0, uv0).xyz;
    frag_color = vec4(c0, 1.0);
}
@end

@program fsq vs_fsq fs_fsq


