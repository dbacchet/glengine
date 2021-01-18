//  encode/decode a float into a rgba value
//  http://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/

// encode a float in range [-1..1) into a vec4 with 8bits per channel (rgba for example)
vec4 encodeDepth(float v) {
    float vn = v * 0.5 + 0.5;
    vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * vn;
    enc = fract(enc);
    enc -= enc.yzww * vec4(1.0 / 255.0, 1.0 / 255.0, 1.0 / 255.0, 0.0);
    return enc;
}

// decode a rgba value into a float in the range [-1..1)
float decodeDepth(vec4 rgba) {
    return dot(rgba, vec4(1.0, 1.0 / 255.0, 1.0 / 65025.0, 1.0 / 16581375.0)) * 2 - 1;
}

// convert depth in linear space
float depth_to_linear_space(float depth, float znear, float zfar) {
    return 2.0 * znear * zfar / (zfar + znear - depth * (zfar - znear));
}

// get the world position from the depth buffer
// pos_clipspace has the pos with x,y,z in the range -1..1, after perspective transfor 
// (i.e. as calculated from `gl_Position = projection * view * model * vertex_pos;`)
vec3 view_pos_from_depth(vec3 pos_clipspace, mat4 inverse_proj) {
    vec4 viewspace_pos = inverse_proj * vec4(pos_clipspace, 1.0);
    // Perspective division
    viewspace_pos /= viewspace_pos.w;

    return viewspace_pos.xyz;
}

