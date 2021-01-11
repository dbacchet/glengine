// vertex shader for PBR materials
uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec4 a_Position;
in vec4 a_Color;
in vec3 a_Normal;
in vec2 a_UV1;
in vec4 a_Tangent;

out vec3 v_Position;
out vec3 v_Normal;
#ifdef HAS_TANGENTS
out vec3 v_Tangent;
out vec3 v_Bitangent;
#endif
out vec2 v_UVCoord1;
out vec2 v_UVCoord2;
out vec4 v_Color;
out vec3 v_eye_pos;

vec4 getPosition()
{
    vec4 pos = vec4(a_Position.xyz, 1.0);
    return pos;
}

vec3 getNormal()
{
    vec3 normal = a_Normal;
    return normalize(normal);
}

#ifdef HAS_TANGENTS
vec3 getTangent()
{
    vec3 tangent = a_Tangent.xyz;
    return normalize(tangent);
}
#endif

void main()
{
    vec4 pos = model * getPosition();
    v_Position = vec3(pos.xyz) / pos.w;


    mat4 normal_matrix = transpose(inverse(model));
    #ifdef HAS_TANGENTS
        vec3 tangent = getTangent();
        vec3 normalW = normalize(vec3(normal_matrix * vec4(getNormal(), 0.0)));
        vec3 tangentW = normalize(vec3(model * vec4(tangent, 0.0)));
        vec3 bitangentW = cross(normalW, tangentW) * a_Tangent.w;
        v_Normal = normalW;
        v_Tangent = tangentW;
        v_Bitangent = bitangentW;
    #else // !HAS_TANGENTS
        v_Normal = normalize(vec3(normal_matrix * vec4(getNormal(), 0.0)));
    #endif

    v_UVCoord1 = vec2(0.0, 0.0);
    v_UVCoord2 = vec2(0.0, 0.0);

    v_UVCoord1 = a_UV1;

    v_Color = a_Color;

    v_eye_pos = inverse(view)[3].xyz;

    gl_Position = projection * view * pos;
}
