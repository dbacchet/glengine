@ctype mat4 math::Matrix4f
@ctype vec4 math::Vector4f
@ctype vec3 math::Vector3f
@ctype vec2 math::Vector2f

@vs vs_flat_textured
uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec4 vertex_pos; // since the pos is a vec3, the last coord will be initialized with 1.0 by the backend when setting the vertex format
in vec4 vertex_color;
in vec3 vertex_normal;
in vec2 vertex_texcoord;

out vec2 uv;

void main() {
    gl_Position = projection * view * model * vertex_pos;
    uv = vertex_texcoord;
}
@end

@fs fs_flat_textured
uniform fs_params {
    vec4 color;
};

uniform sampler2D tex_diffuse;

in vec2 uv;

// uniform uint object_id;

layout(location=0) out vec4 frag_color;

void main() {
    frag_color = texture(tex_diffuse, uv) * color;
}
@end

@program offscreen_flat_textured vs_flat_textured fs_flat_textured



