#pragma once

#include "stock_common.h"

#include <vector>

namespace {

const char *ssao_vs_src =
    R"(#version 330
// vertex attributes
layout (location=0) in vec3 v_position;
layout (location=1) in vec4 v_color;
layout (location=2) in vec3 v_normal;
layout (location=3) in vec2 v_texcoord0;
// outputs
out vec2 texcoord;

void main()
{
    texcoord = v_texcoord0;
    gl_Position = vec4(v_position.x, v_position.y, 0.0, 1.0); // no perspective transformation; assume input already in view space
})";

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
const char *ssao_fs_src =
    R"(#version 330
// uniforms
uniform sampler2D g_position_texture;
uniform sampler2D g_normal_texture;
uniform sampler2D noise_texture;
uniform mat4 u_projection;
uniform vec2 noise_scale; // shuold be (fbsize.w/noise_tex.w, fbsize.h/noise_tex.h) to have perfect tex lookup with textures of same scale
// inputs
in vec2 texcoord;
// outputs
// out float ssao_occlusion;
// parameters
int kernelSize = 64;
uniform float radius = 0.75;
uniform float bias = 0.025;
// tile noise texture over screen based on screen dimensions divided by noise size

layout (location = 0) out vec4 ssao_occlusion;

// samples used to test occlusions
const vec3 samples_[64] = vec3[](
vec3(0.009214,0.028357,0.011744),   vec3(-0.013497,-0.034306,0.019995), vec3(-0.011223,-0.006319,0.000524), vec3(0.023428,0.051367,0.025170),
vec3(0.019346,-0.013266,0.031138),  vec3(-0.030557,0.031596,0.007760),  vec3(0.001808,0.002138,0.003177),   vec3(0.001387,0.020624,0.011748),
vec3(-0.054495,-0.046825,0.017710), vec3(-0.004950,-0.011894,0.012455), vec3(0.012613,-0.010588,0.021011),  vec3(-0.065498,-0.018538,0.072547),
vec3(0.000510,0.000567,0.003645),   vec3(0.060491,-0.008464,0.105874),  vec3(0.011250,-0.010705,0.024158),  vec3(-0.060799,-0.078432,0.100303),
vec3(0.042712,-0.027282,0.093894),  vec3(-0.022921,0.019558,0.000647),  vec3(0.082985,-0.052787,0.104140),  vec3(-0.017679,-0.005066,0.012848),
vec3(-0.007700,0.025123,0.005253),  vec3(-0.011226,0.006113,0.020497),  vec3(-0.022279,-0.002990,0.000235), vec3(0.068467,-0.140935,0.123587),
vec3(0.024176,0.029790,0.042241),   vec3(0.079604,-0.150298,0.099236),  vec3(0.001274,0.002751,0.001553),   vec3(-0.140376,-0.111517,0.109307),
vec3(0.027361,-0.073462,0.077265),  vec3(0.036825,0.052897,0.072865),   vec3(0.058904,0.047293,0.063670),   vec3(0.261433,0.099157,0.098504),
vec3(0.189585,-0.042820,0.188684),  vec3(-0.122516,-0.137717,0.072885), vec3(-0.088046,0.075440,0.002412),  vec3(0.050743,-0.076847,0.030228),
vec3(-0.036384,0.038649,0.063895),  vec3(-0.184577,-0.143545,0.060559), vec3(0.015326,0.092286,0.084143),   vec3(-0.140311,-0.104943,0.146923),
vec3(-0.061537,-0.242710,0.236099), vec3(0.212871,-0.161008,0.057775),  vec3(0.143753,0.019286,0.031377),   vec3(0.187666,-0.393953,0.091848),
vec3(-0.074022,-0.310141,0.067743), vec3(-0.173021,0.306464,0.231160),  vec3(-0.235081,0.248259,0.116597),  vec3(0.018515,-0.016222,0.032047),
vec3(-0.192486,-0.299465,0.139539), vec3(0.036843,0.311018,0.382776),   vec3(0.146672,0.278042,0.331391),   vec3(-0.147242,0.082309,0.191160),
vec3(0.271752,-0.239289,0.455959),  vec3(-0.024143,-0.008788,0.024198), vec3(0.246711,-0.507258,0.414563),  vec3(0.553738,0.040646,0.397077),
vec3(-0.474526,-0.411109,0.173680), vec3(0.339285,0.295373,0.607759),   vec3(0.032039,-0.048462,0.510512),  vec3(0.123045,0.005883,0.191955),
vec3(0.118399,0.159517,0.213312),   vec3(0.448766,-0.195609,0.439006),  vec3(0.226670,-0.640701,0.026292),  vec3(-0.501758,-0.642840,0.231969)
);

void main()
{
    // get input for SSAO algorithm
    vec3 frag_pos = texture(g_position_texture, texcoord).xyz;
    vec3 normal = normalize(texture(g_normal_texture, texcoord).rgb);
    vec3 random_vec = normalize(texture(noise_texture, texcoord * noise_scale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples_[i]; // from tangent to view-space
        sample = frag_pos + sample * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = u_projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sample_depth = texture(g_position_texture, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    ssao_occlusion = vec4(occlusion,occlusion,occlusion,1);
})";



}

namespace glengine {
namespace stock_shaders {

// //// //
// ssao //
// //// //

static std::vector<const char*> ssao_vs_srcs = {ssao_vs_src};
static std::vector<const char*> ssao_fs_srcs = {ssao_fs_src};

} //
} // 
