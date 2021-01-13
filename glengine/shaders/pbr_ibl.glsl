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
/* #define USE_PUNCTUAL */
#define USE_IBL
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

@vs vs_pbr_ibl

@include_block common_defines

@include pbr_vs.glsl.inc

@end

@fs fs_pbr_ibl

@include_block common_defines

@include pbr_fs.glsl.inc

@end


@program offscreen_pbr_ibl vs_pbr_ibl fs_pbr_ibl


// alternative PBR shader adapted from here: https://github.com/oframe/ogl/blob/master/examples/pbr.html
@vs vs_pbr_alternative
uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec4 a_Position;
in vec4 a_Color;
in vec3 a_Normal;
in vec2 a_UV1;

out vec2 vUv;
out vec3 vNormal;
out vec3 vMPos;
out vec3 v_eye_pos;
void main() {
    vec4 pos = model * vec4(a_Position.xyz,1);
    vMPos = vec3(pos.xyz) / pos.w;
    vUv = a_UV1;
    mat4 normal_matrix = transpose(inverse(model));
    vNormal = normalize(normal_matrix * vec4(normalize(a_Normal),1)).xyz;
    v_eye_pos = inverse(view)[3].xyz;
    gl_Position = projection * view * pos;
}
@end


@fs fs_pbr_alternative

precision highp float;
uniform sampler2D tBaseColor;
uniform sampler2D tRMO;
uniform sampler2D tNormal;
uniform sampler2D tEmissive;
uniform sampler2D tLUT;
uniform sampler2D tEnvDiffuse;
uniform sampler2D tEnvSpecular;
/* sampler2D tOpacity; */
uniform fs_params_alternative {
mat4 viewMatrix;
vec3 uBaseColor;
float uAlpha;
float uMetallic;
float uRoughness;
float uOcclusion;
float uNormalScale;
float uNormalUVScale;
float uEmissive;
float uEnvSpecular;
vec3 uLightDirection;
vec3 uLightColor;
};

in vec2 vUv;
in vec3 vNormal;
in vec3 vMPos;
in vec3 v_eye_pos;
out vec4 FragColor;
const float PI = 3.14159265359;
const float RECIPROCAL_PI = 0.31830988618;
const float RECIPROCAL_PI2 = 0.15915494;
const float LN2 = 0.6931472;
const float ENV_LODS = 6.0;
vec4 SRGBtoLinear(vec4 srgb) {
    vec3 linOut = pow(srgb.xyz, vec3(2.2));
    return vec4(linOut, srgb.w);;
}
vec4 RGBMToLinear(in vec4 value) {
    float maxRange = 6.0;
    return vec4(value.xyz * value.w * maxRange, 1.0);
}
vec3 linearToSRGB(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}
vec3 getNormal() {
    vec3 pos_dx = dFdx(vMPos.xyz);
    vec3 pos_dy = dFdy(vMPos.xyz);
    vec2 tex_dx = dFdx(vUv);
    vec2 tex_dy = dFdy(vUv);
    vec3 t = normalize(pos_dx * tex_dy.t - pos_dy * tex_dx.t);
    vec3 b = normalize(-pos_dx * tex_dy.s + pos_dy * tex_dx.s);
    mat3 tbn = mat3(t, b, normalize(vNormal));
    vec3 n = texture(tNormal, vUv * uNormalUVScale).rgb * 2.0 - 1.0;
    n.xy *= uNormalScale;
    vec3 normal = normalize(tbn * n);
    // Get world normal from view normal (normalMatrix * normal)
    return normalize((vec4(normal, 0.0) * viewMatrix).xyz);
}
vec3 specularReflection(vec3 specularEnvR0, vec3 specularEnvR90, float VdH) {
    return specularEnvR0 + (specularEnvR90 - specularEnvR0) * pow(clamp(1.0 - VdH, 0.0, 1.0), 5.0);
}
float geometricOcclusion(float NdL, float NdV, float roughness) {
    float r = roughness;
    float attenuationL = 2.0 * NdL / (NdL + sqrt(r * r + (1.0 - r * r) * (NdL * NdL)));
    float attenuationV = 2.0 * NdV / (NdV + sqrt(r * r + (1.0 - r * r) * (NdV * NdV)));
    return attenuationL * attenuationV;
}
float microfacetDistribution(float roughness, float NdH) {
    float roughnessSq = roughness * roughness;
    float f = (NdH * roughnessSq - NdH) * NdH + 1.0;
    return roughnessSq / (PI * f * f);
}
vec2 cartesianToPolar(vec3 n) {
    vec2 uv;
    uv.x = atan(n.z, n.x) * RECIPROCAL_PI2 + 0.5;
    uv.y = asin(n.y) * RECIPROCAL_PI + 0.5;
    return uv;
}
void getIBLContribution(inout vec3 diffuse, inout vec3 specular, float NdV, float roughness, vec3 n, vec3 reflection, vec3 diffuseColor, vec3 specularColor) {
    vec3 brdf = SRGBtoLinear(texture(tLUT, vec2(NdV, roughness))).rgb;
    vec3 diffuseLight = RGBMToLinear(texture(tEnvDiffuse, cartesianToPolar(n))).rgb;
    // Sample 2 levels and mix between to get smoother degradation
    float blend = roughness * ENV_LODS;
    float level0 = floor(blend);
    float level1 = min(ENV_LODS, level0 + 1.0);
    blend -= level0;
    
    // Sample the specular env map atlas depending on the roughness value
    vec2 uvSpec = cartesianToPolar(reflection);
    uvSpec.y /= 2.0;
    vec2 uv0 = uvSpec;
    vec2 uv1 = uvSpec;
    uv0 /= pow(2.0, level0);
    uv0.y += 1.0 - exp(-LN2 * level0);
    uv1 /= pow(2.0, level1);
    uv1.y += 1.0 - exp(-LN2 * level1);
    vec3 specular0 = RGBMToLinear(texture(tEnvSpecular, uv0)).rgb;
    vec3 specular1 = RGBMToLinear(texture(tEnvSpecular, uv1)).rgb;
    vec3 specularLight = mix(specular0, specular1, blend);
    diffuse = diffuseLight * diffuseColor;
    
    // Bit of extra reflection for smooth materials
    float reflectivity = pow((1.0 - roughness), 2.0) * 0.05;
    specular = specularLight * (specularColor * brdf.x + brdf.y + reflectivity);
    specular *= uEnvSpecular;
}
void main() {

    vec3 baseColor = SRGBtoLinear(texture(tBaseColor, vUv)).rgb * uBaseColor;
    // RMO map packed as rgb = [occlusion, roughness, metallic]
    vec4 rmaSample = texture(tRMO, vUv);
    float roughness = clamp(rmaSample.g * uRoughness, 0.04, 1.0);
    float metallic = clamp(rmaSample.b * uMetallic, 0.04, 1.0);
    vec3 f0 = vec3(0.04);
    vec3 diffuseColor = baseColor * (vec3(1.0) - f0) * (1.0 - metallic);
    vec3 specularColor = mix(f0, baseColor, metallic);
    vec3 specularEnvR0 = specularColor;
    vec3 specularEnvR90 = vec3(clamp(max(max(specularColor.r, specularColor.g), specularColor.b) * 25.0, 0.0, 1.0));
    vec3 N = getNormal();
    vec3 V = normalize(v_eye_pos - vMPos);
    vec3 L = normalize(uLightDirection);
    vec3 H = normalize(L + V);
    vec3 reflection = normalize(reflect(-V, N));
    float NdL = clamp(dot(N, L), 0.001, 1.0);
    float NdV = clamp(abs(dot(N, V)), 0.001, 1.0);
    float NdH = clamp(dot(N, H), 0.0, 1.0);
    float LdH = clamp(dot(L, H), 0.0, 1.0);
    float VdH = clamp(dot(V, H), 0.0, 1.0);
    vec3 F = specularReflection(specularEnvR0, specularEnvR90, VdH);
    float G = geometricOcclusion(NdL, NdV, roughness);
    float D = microfacetDistribution(roughness, NdH);
    vec3 diffuseContrib = (1.0 - F) * (diffuseColor / PI);
    vec3 specContrib = F * G * D / (4.0 * NdL * NdV);
    
    // Shading based off lights
    vec3 color = NdL * uLightColor * (diffuseContrib + specContrib);
    // Get base alpha
    float alpha = 1.0;
    /* alpha *= texture(tOpacity, vUv).g; */
    // Add lights spec to alpha for reflections on transparent surfaces (glass)
    alpha = max(alpha, max(max(specContrib.r, specContrib.g), specContrib.b));
    // Calculate IBL lighting
    vec3 diffuseIBL;
    vec3 specularIBL;
    getIBLContribution(diffuseIBL, specularIBL, NdV, roughness, N, reflection, diffuseColor, specularColor);
    // Add IBL on top of color
    color += diffuseIBL + specularIBL;
    // Add IBL spec to alpha for reflections on transparent surfaces (glass)
    alpha = max(alpha, max(max(specularIBL.r, specularIBL.g), specularIBL.b));
    // Multiply occlusion
    color = mix(color, color /* rmaSample.r*/, uOcclusion);
    // Add emissive on top
    vec3 emissive = SRGBtoLinear(texture(tEmissive, vUv)).rgb * uEmissive;
    color += emissive;
    // Convert to sRGB to display
    FragColor.rgb = linearToSRGB(color);
    
    // Apply uAlpha uniform at the end to overwrite any specular additions on transparent surfaces
    FragColor.a = alpha * uAlpha;
}
@end


@program offscreen_pbr_alternative_ibl vs_pbr_alternative fs_pbr_alternative

