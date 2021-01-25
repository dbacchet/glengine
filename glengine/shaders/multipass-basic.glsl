@ctype mat4 math::Matrix4f
@ctype vec2 math::Matrix2f

// shaders for rendering a fullscreen-quad in default pass
@vs vs_fsq
@msl_options flip_vert_y

in vec2 pos;
out vec2 uv0;

void main() {
    gl_Position = vec4(pos*2.0-1.0, 0.5, 1.0);
    uv0 = pos;
}
@end


@fs fs_fsq
@include common.glsl.inc
uniform fsq_params {
    float debug_view;
    float znear;
    float zfar;
};
uniform sampler2D tex0;
uniform sampler2D tex_normal;
uniform sampler2D tex_depth;
uniform sampler2D tex_ssao;

in vec2 uv0;

out vec4 frag_color;

void main() {
    vec3 col = texture(tex0, uv0).xyz;
    // apply ssao
    col *= texture(tex_ssao, uv0).x;

    if (debug_view>0.5) {
        if (uv0.x<=0.5 && uv0.y>0.5) {
            // top left: normal color
            col = texture(tex0, vec2((uv0.x)*2.0,(uv0.y-0.5)*2.0)).rgb;
        } else if (uv0.x>0.5 && uv0.y>0.5) {
            // top right: normal
            col = texture(tex_normal, vec2((uv0.x-0.5)*2.0,(uv0.y-0.5)*2.0)).rgb;
        } else if (uv0.x<=0.5 && uv0.y<0.5) {
            // bottom left: depth
            vec4 depth_col = texture(tex_depth, vec2((uv0.x)*2.0,(uv0.y)*2.0));
            float depth = decodeDepth(depth_col);
            float z_eye = 2.0 * znear / (zfar + znear - depth * (zfar - znear)); // scaled down by f to have it in the range 0..1
            col = vec3(z_eye,z_eye,z_eye);
        } else if (uv0.x>0.5 && uv0.y<0.5) {
            // bottom right: ssao
            col = texture(tex_ssao, vec2((uv0.x-0.5)*2.0,(uv0.y)*2.0)).rgb;
        }
    }
    frag_color = vec4(col, 1.0);
}
@end

@program fsq vs_fsq fs_fsq


