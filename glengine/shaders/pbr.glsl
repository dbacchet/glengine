@ctype mat4 math::Matrix4f
@ctype vec4 math::Vector4f
@ctype vec3 math::Vector3f
@ctype vec2 math::Vector3f

@block common_defines
// defines to enable the required features
// #define HAS_TANGENTS
#define MATERIAL_METALLICROUGHNESS
#define HAS_BASE_COLOR_MAP
#define HAS_METALLIC_ROUGHNESS_MAP
#define HAS_NORMAL_MAP
#define HAS_EMISSIVE_MAP
#define HAS_OCCLUSION_MAP
#define USE_PUNCTUAL
// #define USE_IBL
#define USE_HDR

// #define DEBUG_OUTPUT
// #define DEBUG_METALLIC // OK
// #define DEBUG_ROUGHNESS // OK
// #define DEBUG_NORMAL // OK
// #define DEBUG_TANGENT // OK
// #define DEBUG_BITANGENT // OK
// #define DEBUG_BASECOLOR // OK

// #define ALPHAMODE_OPAQUE
// #define MATERIAL_UNLIT
@end

@vs vs_pbr

@include_block common_defines

@include pbr_vs.glsl.inc

@end







@fs fs_pbr

@include_block common_defines

@include pbr_fs.glsl.inc

@end


@program offscreen_pbr vs_pbr fs_pbr

