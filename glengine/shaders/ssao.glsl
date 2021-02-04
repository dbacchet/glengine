@ctype mat4 math::Matrix4f
@ctype vec2 math::Vector2f
@ctype vec4 math::Vector4f

// shaders for calculating screen space ambient occlusion
@vs vs_ssao
in vec2 pos;
out vec2 uv0;

void main() {
    gl_Position = vec4(pos * 2.0 - 1.0, 0.5, 1.0);
    uv0 = pos;
}
@end

@fs fs_ssao
@include common.glsl.inc
uniform ssao_params {
    mat4 u_view;
    mat4 u_projection;
    mat4 u_inv_projection;
    float radius;
    vec4 noise_scale;
    float bias;
};
uniform sampler2D tex_normal_sampler;
uniform sampler2D tex_depth_sampler;
uniform sampler2D tex_random_sampler;

in vec2 uv0;

out vec4 frag_color;

/* vec3 normal_from_depth(float depth, vec2 texcoords) { */
/*  */
/*     const vec2 offset1 = vec2(0.0, 0.001); */
/*     const vec2 offset2 = vec2(0.001, 0.0); */
/*  */
/*     vec4 depth1_rgba = texture(tex_depth_sampler, texcoords + offset1); */
/*     vec4 depth2_rgba = texture(tex_depth_sampler, texcoords + offset2); */
/*  */
/*     float depth1 = depth_to_linear_space(decodeDepth(depth1_rgba), znear, zfar); */
/*     float depth2 = depth_to_linear_space(decodeDepth(depth2_rgba), znear, zfar); */
/*  */
/*     vec3 p1 = vec3(offset1, depth1 - depth); */
/*     vec3 p2 = vec3(offset2, depth2 - depth); */
/*  */
/*     vec3 normal = cross(p1, p2); */
/*     normal.z = -normal.z; */
/*  */
/*     return normalize(normal); */
/* } */

void main() {
    // SSAO samples can be created with a function like this in c++:
    // std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    // std::mt19937 gen(12345678);
    // for (unsigned int i = 0; i < num_ssao_kernel_samples; ++i) {
    //     math::Vector3f sample(randomFloats(gen) * 2.0 - 1.0, randomFloats(gen) * 2.0 - 1.0, randomFloats(gen));
    //     math::normalize(sample);
    //     sample *= randomFloats(gen);
    //     float scale = float(i) / num_ssao_kernel_samples;
    //     // scale samples s.t. they're more aligned to center of kernel
    //     scale = math::utils::lerp(0.1f, 1.0f, scale * scale);
    //     sample *= scale;
    //     ssaoKernel[i] = sample;
    //     printf("vec3(%f,%f,%f)\n",sample.x,sample.y,sample.z);
    // }
    const int samples = 64;
    const vec3 sample_sphere[samples] = vec3[](
        vec3(0.009214, 0.028357, 0.011744), vec3(-0.013497, -0.034306, 0.019995), vec3(-0.011223, -0.006319, 0.000524),
        vec3(0.023428, 0.051367, 0.025170), vec3(0.019346, -0.013266, 0.031138), vec3(-0.030557, 0.031596, 0.007760),
        vec3(0.001808, 0.002138, 0.003177), vec3(0.001387, 0.020624, 0.011748), vec3(-0.054495, -0.046825, 0.017710),
        vec3(-0.004950, -0.011894, 0.012455), vec3(0.012613, -0.010588, 0.021011), vec3(-0.065498, -0.018538, 0.072547),
        vec3(0.000510, 0.000567, 0.003645), vec3(0.060491, -0.008464, 0.105874), vec3(0.011250, -0.010705, 0.024158),
        vec3(-0.060799, -0.078432, 0.100303), vec3(0.042712, -0.027282, 0.093894), vec3(-0.022921, 0.019558, 0.000647),
        vec3(0.082985, -0.052787, 0.104140), vec3(-0.017679, -0.005066, 0.012848), vec3(-0.007700, 0.025123, 0.005253),
        vec3(-0.011226, 0.006113, 0.020497), vec3(-0.022279, -0.002990, 0.000235), vec3(0.068467, -0.140935, 0.123587),
        vec3(0.024176, 0.029790, 0.042241), vec3(0.079604, -0.150298, 0.099236), vec3(0.001274, 0.002751, 0.001553),
        vec3(-0.140376, -0.111517, 0.109307), vec3(0.027361, -0.073462, 0.077265), vec3(0.036825, 0.052897, 0.072865),
        vec3(0.058904, 0.047293, 0.063670), vec3(0.261433, 0.099157, 0.098504), vec3(0.189585, -0.042820, 0.188684),
        vec3(-0.122516, -0.137717, 0.072885), vec3(-0.088046, 0.075440, 0.002412), vec3(0.050743, -0.076847, 0.030228),
        vec3(-0.036384, 0.038649, 0.063895), vec3(-0.184577, -0.143545, 0.060559), vec3(0.015326, 0.092286, 0.084143),
        vec3(-0.140311, -0.104943, 0.146923), vec3(-0.061537, -0.242710, 0.236099), vec3(0.212871, -0.161008, 0.057775),
        vec3(0.143753, 0.019286, 0.031377), vec3(0.187666, -0.393953, 0.091848), vec3(-0.074022, -0.310141, 0.067743),
        vec3(-0.173021, 0.306464, 0.231160), vec3(-0.235081, 0.248259, 0.116597), vec3(0.018515, -0.016222, 0.032047),
        vec3(-0.192486, -0.299465, 0.139539), vec3(0.036843, 0.311018, 0.382776), vec3(0.146672, 0.278042, 0.331391),
        vec3(-0.147242, 0.082309, 0.191160), vec3(0.271752, -0.239289, 0.455959), vec3(-0.024143, -0.008788, 0.024198),
        vec3(0.246711, -0.507258, 0.414563), vec3(0.553738, 0.040646, 0.397077), vec3(-0.474526, -0.411109, 0.173680),
        vec3(0.339285, 0.295373, 0.607759), vec3(0.032039, -0.048462, 0.510512), vec3(0.123045, 0.005883, 0.191955),
        vec3(0.118399, 0.159517, 0.213312), vec3(0.448766, -0.195609, 0.439006), vec3(0.226670, -0.640701, 0.026292),
        vec3(-0.501758, -0.642840, 0.231969));

    // get pos from depth texture
    vec4 depth_rgba = texture(tex_depth_sampler, uv0);
    float depth = decodeDepth(depth_rgba);
    if (depth>0.999) { // ignore the far plane
        frag_color = vec4(1,1,1,1);
        return;
    }
    vec3 pos_clipspace = vec3(uv0 * 2 - 1, depth);
    vec3 frag_pos = view_pos_from_depth(pos_clipspace, u_inv_projection);
    /* vec3 normal = normalize(texture(g_normal_texture, uv0).rgb); */
    vec3 normal_world = texture(tex_normal_sampler, uv0).xyz * 2 - 1;
    vec3 normal = normalize(mat3(u_view) * normal_world); // normal in view space
    vec3 random_vec = normalize(texture(tex_random_sampler, uv0 * noise_scale.xy).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < samples; ++i) {
        // get sample position
        vec3 smpl = TBN * sample_sphere[i]; // align with the normal in view-space
        smpl = frag_pos + smpl * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(smpl, 1.0);
        offset = u_projection * offset;      // from view to clip-space
        offset.xyz /= offset.w;              // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        // get sample depth
        vec4 sample_depth_rgba = texture(tex_depth_sampler, offset.xy);
        float sample_depth_cs = decodeDepth(sample_depth_rgba);
        vec3 sample_pos_vs = view_pos_from_depth(vec3(offset.xy,sample_depth_cs), u_inv_projection);
        float sample_depth = sample_pos_vs.z;

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth >= smpl.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / samples);

    frag_color = vec4(occlusion, occlusion, occlusion, 1);
}
@end

@program ssao vs_ssao fs_ssao

