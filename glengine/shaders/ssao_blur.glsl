@ctype mat4 math::Matrix4f
@ctype vec2 math::Matrix2f


@vs vs_blur
in vec2 pos;
out vec2 uv0;

void main() {
    gl_Position = vec4(pos*2.0-1.0, 0.5, 1.0);
    uv0 = pos;
}
@end


@fs fs_blur
in vec2 uv0;
  
uniform sampler2D tex;

out vec4 frag_color;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(tex, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(tex, uv0 + offset).r;
        }
    }
    frag_color = vec4(vec3(result)/(4*4), 1);
}  

@end

@program ssao_blur vs_blur fs_blur


