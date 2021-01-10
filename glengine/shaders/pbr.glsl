@ctype mat4 math::Matrix4f
@ctype vec4 math::Vector4f
@ctype vec3 math::Vector3f
@ctype vec2 math::Vector3f

@block common_defines
// defines to enable the required features
#define MATERIAL_METALLICROUGHNESS
#define HAS_BASE_COLOR_MAP
#define HAS_METALLIC_ROUGHNESS_MAP
#define HAS_NORMAL_MAP
#define HAS_EMISSIVE_MAP
#define HAS_OCCLUSION_MAP
#define USE_PUNCTUAL
// #define DEBUG_OUTPUT
// #define DEBUG_NORMAL
// #define DEBUG_METALLIC
// #define DEBUG_ROUGHNESS
// #define DEBUG_NORMAL // OK
// #define DEBUG_TANGENT
// #define DEBUG_BASECOLOR // OK

// #define DEBUG_BASECOLOR
// #define ALPHAMODE_OPAQUE
/* #define MATERIAL_UNLIT */
@end

@vs vs_pbr

@include_block common_defines

uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

// in vec4 vertex_pos;
// in vec4 vertex_color;
// in vec3 vertex_normal;
// in vec2 vertex_texcoord;

in vec4 a_Position;
in vec4 a_Color;
in vec3 a_Normal;
in vec2 a_UV1;

out vec3 v_Position;
out vec3 v_Normal;
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

void main()
{
    vec4 pos = model * getPosition();
    v_Position = vec3(pos.xyz) / pos.w;

    // v_Normal = normalize(vec3(u_NormalMatrix * vec4(getNormal(), 0.0)));
    v_Normal = normalize(vec3(model * vec4(getNormal(), 0.0)));

    v_UVCoord1 = vec2(0.0, 0.0);
    v_UVCoord2 = vec2(0.0, 0.0);

    v_UVCoord1 = a_UV1;

    v_Color = a_Color;

    v_eye_pos = inverse(view)[3].xyz;

    gl_Position = projection * view * pos;
}
@end







@fs fs_pbr

@include_block common_defines

precision highp float;

in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_UVCoord1;
in vec2 v_UVCoord2;
in vec4 v_Color;
in vec3 v_eye_pos;

@include tonemapping.glsl.inc
@include textures.glsl.inc
@include functions.glsl.inc
@include brdf.glsl.inc
@include punctual.glsl.inc
// #include <ibl.glsl>

out vec4 g_finalColor;

uniform Light
{
    vec3 light_position;
    float light_intensity;
    float light_range;
    vec3 light_color;
    vec3 light_direction;
    //
    //
    // float light_innerConeCos;
    //
    // float light_outerConeCos;
    // int light_type;
};
const int light_type = LightType_Directional;

#ifdef USE_PUNCTUAL
#define LIGHT_COUNT 1
// uniform Light u_Lights[LIGHT_COUNT];
#endif

uniform fs_params {
// Metallic Roughness
float u_MetallicFactor;
float u_RoughnessFactor;
vec4 u_BaseColorFactor;
/*  */
/* // Specular Glossiness */
/* vec3 u_SpecularFactor; */
/* vec4 u_DiffuseFactor; */
/* float u_GlossinessFactor; */
/*  */
/* // Sheen */
/* float u_SheenIntensityFactor; */
/* vec3 u_SheenColorFactor; */
/* float u_SheenRoughness; */
/*  */
/* // Clearcoat */
/* float u_ClearcoatFactor; */
/* float u_ClearcoatRoughnessFactor; */
/*  */
/* // Specular */
/* float u_MetallicRoughnessSpecularFactor; */
/*  */
/* // Anisotropy */
/* float u_Anisotropy; */
/* vec3 u_AnisotropyDirection; */
/*  */
/* // Subsurface */
/* float u_SubsurfaceScale; */
/* float u_SubsurfaceDistortion; */
/* float u_SubsurfacePower; */
/* vec3 u_SubsurfaceColorFactor; */
/* float u_SubsurfaceThicknessFactor; */
/*  */
/* // Thin Film */
/* float u_ThinFilmFactor; */
/* float u_ThinFilmThicknessMinimum; */
/* float u_ThinFilmThicknessMaximum; */
/*  */
/* // IOR (in .x) and the corresponding f0 (in .y) */
/* vec2 u_IOR_and_f0; */
/*  */
/* // Thickness */
/* float u_Thickness; */
/*  */
/* // Absorption */
/* vec3 u_AbsorptionColor; */
/*  */
/* // Transmission */
/* float u_Transmission; */
/*  */
/* // Alpha mode */
/* float u_AlphaCutoff; */

/* vec3 u_Camera; */
};

struct MaterialInfo
{
    float perceptualRoughness;      // roughness value, as authored by the model creator (input to shader)
    vec3 f0;                        // full reflectance color (n incidence angle)

    float alphaRoughness;           // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 albedoColor;

    vec3 f90;                       // reflectance color at grazing angle
    float metallic;

    vec3 n;
    vec3 baseColor; // getBaseColor()

    float sheenIntensity;
    vec3 sheenColor;
    float sheenRoughness;

    float anisotropy;

    vec3 clearcoatF0;
    vec3 clearcoatF90;
    float clearcoatFactor;
    vec3 clearcoatNormal;
    float clearcoatRoughness;

    float subsurfaceScale;
    float subsurfaceDistortion;
    float subsurfacePower;
    vec3 subsurfaceColor;
    float subsurfaceThickness;

    float thinFilmFactor;
    float thinFilmThickness;

    float thickness;

    vec3 absorption;

    float transmission;
};

// Get normal, tangent and bitangent vectors.
NormalInfo getNormalInfo(vec3 v)
{
    vec2 UV = getNormalUV();
    vec3 uv_dx = dFdx(vec3(UV, 0.0));
    vec3 uv_dy = dFdy(vec3(UV, 0.0));

    vec3 t_ = (uv_dy.t * dFdx(v_Position) - uv_dx.t * dFdy(v_Position)) /
        (uv_dx.s * uv_dy.t - uv_dy.s * uv_dx.t);

    vec3 n, t, b, ng;

    // Compute geometrical TBN:
    ng = normalize(v_Normal);
    t = normalize(t_ - ng * dot(ng, t_));
    b = cross(ng, t);

    // For a back-facing surface, the tangential basis vectors are negated.
    float facing = step(0.0, dot(v, ng)) * 2.0 - 1.0;
    t *= facing;
    b *= facing;
    ng *= facing;

    // Due to anisoptry, the tangent can be further rotated around the geometric normal.
    vec3 direction;
    #ifdef MATERIAL_ANISOTROPY
        #ifdef HAS_ANISOTROPY_DIRECTION_MAP
            direction = texture(u_AnisotropyDirectionSampler, getAnisotropyDirectionUV()).xyz * 2.0 - vec3(1.0);
        #else
            direction = u_AnisotropyDirection;
        #endif
    #else
        direction = vec3(1.0, 0.0, 0.0);
    #endif
    t = mat3(t, b, ng) * normalize(direction);
    b = normalize(cross(ng, t));

    // Compute pertubed normals:
    #ifdef HAS_NORMAL_MAP
        n = texture(u_NormalSampler, UV).rgb * 2.0 - vec3(1.0);
        n *= vec3(u_NormalScale, u_NormalScale, 1.0);
        n = mat3(t, b, ng) * normalize(n);
    #else
        n = ng;
    #endif

    NormalInfo info;
    info.ng = ng;
    info.t = t;
    info.b = b;
    info.n = n;
    return info;
}
 
vec4 getBaseColor()
{
    vec4 baseColor = vec4(1, 1, 1, 1);

    #if defined(MATERIAL_SPECULARGLOSSINESS)
        baseColor = u_DiffuseFactor;
    #elif defined(MATERIAL_METALLICROUGHNESS)
        baseColor = u_BaseColorFactor;
    #endif

    #if defined(MATERIAL_SPECULARGLOSSINESS) && defined(HAS_DIFFUSE_MAP)
        baseColor *= sRGBToLinear(texture(u_DiffuseSampler, getDiffuseUV()));
    #elif defined(MATERIAL_METALLICROUGHNESS) && defined(HAS_BASE_COLOR_MAP)
        baseColor *= sRGBToLinear(texture(u_BaseColorSampler, getBaseColorUV()));
    #endif

    return baseColor * getVertexColor();
}

// MaterialInfo getSpecularGlossinessInfo(MaterialInfo info)
// {
//     info.f0 = u_SpecularFactor;
//     info.perceptualRoughness = u_GlossinessFactor;
// 
// #ifdef HAS_SPECULAR_GLOSSINESS_MAP
//     vec4 sgSample = sRGBToLinear(texture(u_SpecularGlossinessSampler, getSpecularGlossinessUV()));
//     info.perceptualRoughness *= sgSample.a ; // glossiness to roughness
//     info.f0 *= sgSample.rgb; // specular
// #endif // ! HAS_SPECULAR_GLOSSINESS_MAP
// 
//     info.perceptualRoughness = 1.0 - info.perceptualRoughness; // 1 - glossiness
//     info.albedoColor = info.baseColor.rgb * (1.0 - max(max(info.f0.r, info.f0.g), info.f0.b));
// 
//     return info;
// }
// 
// // KHR_extension_specular alters f0 on metallic materials based on the specular factor specified in the extention
// float getMetallicRoughnessSpecularFactor()
// {
//     //F0 = 0.08 * specularFactor * specularTexture
// #ifdef HAS_METALLICROUGHNESS_SPECULAROVERRIDE_MAP
//     vec4 specSampler =  texture(u_MetallicRoughnessSpecularSampler, getMetallicRoughnessSpecularUV());
//     return 0.08 * u_MetallicRoughnessSpecularFactor * specSampler.a;
// #endif
//     return  0.08 * u_MetallicRoughnessSpecularFactor;
// }

MaterialInfo getMetallicRoughnessInfo(MaterialInfo info, float f0_ior)
{
    info.metallic = u_MetallicFactor;
    info.perceptualRoughness = u_RoughnessFactor;

#ifdef HAS_METALLIC_ROUGHNESS_MAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    vec4 mrSample = texture(u_MetallicRoughnessSampler, getMetallicRoughnessUV());
    info.perceptualRoughness *= mrSample.g;
    info.metallic *= mrSample.b;
#endif

#ifdef MATERIAL_METALLICROUGHNESS_SPECULAROVERRIDE
    // Overriding the f0 creates unrealistic materials if the IOR does not match up.
    vec3 f0 = vec3(getMetallicRoughnessSpecularFactor());
#else
    // Achromatic f0 based on IOR.
    vec3 f0 = vec3(f0_ior);
#endif

    info.albedoColor = mix(info.baseColor.rgb * (vec3(1.0) - f0),  vec3(0), info.metallic);
    info.f0 = mix(f0, info.baseColor.rgb, info.metallic);

    return info;
}

// MaterialInfo getSheenInfo(MaterialInfo info)
// {
//     info.sheenColor = u_SheenColorFactor;
//     info.sheenIntensity = u_SheenIntensityFactor;
//     info.sheenRoughness = u_SheenRoughness;
// 
//     #ifdef HAS_SHEEN_COLOR_INTENSITY_MAP
//         vec4 sheenSample = texture(u_SheenColorIntensitySampler, getSheenUV());
//         info.sheenColor *= sheenSample.xyz;
//         info.sheenIntensity *= sheenSample.w;
//     #endif
// 
//     return info;
// }
// 
// #ifdef MATERIAL_SUBSURFACE
// MaterialInfo getSubsurfaceInfo(MaterialInfo info)
// {
//     info.subsurfaceScale = u_SubsurfaceScale;
//     info.subsurfaceDistortion = u_SubsurfaceDistortion;
//     info.subsurfacePower = u_SubsurfacePower;
//     info.subsurfaceColor = u_SubsurfaceColorFactor;
//     info.subsurfaceThickness = u_SubsurfaceThicknessFactor;
// 
//     #ifdef HAS_SUBSURFACE_COLOR_MAP
//         info.subsurfaceColor *= texture(u_SubsurfaceColorSampler, getSubsurfaceColorUV()).rgb;
//     #endif
// 
//     #ifdef HAS_SUBSURFACE_THICKNESS_MAP
//         info.subsurfaceThickness *= texture(u_SubsurfaceThicknessSampler, getSubsurfaceThicknessUV()).r;
//     #endif
// 
//     return info;
// }
// #endif
// 
// vec3 getThinFilmF0(vec3 f0, vec3 f90, float NdotV, float thinFilmFactor, float thinFilmThickness)
// {
//     if (thinFilmFactor == 0.0)
//     {
//         // No thin film applied.
//         return f0;
//     }
// 
//     vec3 lutSample = texture(u_ThinFilmLUT, vec2(thinFilmThickness, NdotV)).rgb - 0.5;
//     vec3 intensity = thinFilmFactor * 4.0 * f0 * (1.0 - f0);
//     return clamp(intensity * lutSample, 0.0, 1.0);
// }
// 
// #ifdef MATERIAL_THIN_FILM
// MaterialInfo getThinFilmInfo(MaterialInfo info)
// {
//     info.thinFilmFactor = u_ThinFilmFactor;
//     info.thinFilmThickness = u_ThinFilmThicknessMaximum / 1200.0;
// 
//     #ifdef HAS_THIN_FILM_MAP
//         info.thinFilmFactor *= texture(u_ThinFilmSampler, getThinFilmUV()).r;
//     #endif
// 
//     #ifdef HAS_THIN_FILM_THICKNESS_MAP
//         float thicknessSampled = texture(u_ThinFilmThicknessSampler, getThinFilmThicknessUV()).g;
//         float thickness = mix(u_ThinFilmThicknessMinimum / 1200.0, u_ThinFilmThicknessMaximum / 1200.0, thicknessSampled);
//         info.thinFilmThickness = thickness;
//     #endif
// 
//     return info;
// }
// #endif
// 
// MaterialInfo getTransmissionInfo(MaterialInfo info)
// {
//     info.transmission = u_Transmission;
//     return info;
// }

MaterialInfo getThicknessInfo(MaterialInfo info)
{
    info.thickness = 1.0;

    #ifdef MATERIAL_THICKNESS
    info.thickness = u_Thickness;

    #ifdef HAS_THICKNESS_MAP
    info.thickness *= texture(u_ThicknessSampler, getThicknessUV()).r;
    #endif

    #endif

    return info;
}

MaterialInfo getAbsorptionInfo(MaterialInfo info)
{
    info.absorption = vec3(0.0);

    #ifdef MATERIAL_ABSORPTION
    info.absorption = u_AbsorptionColor;
    #endif

    return info;
}

// MaterialInfo getAnisotropyInfo(MaterialInfo info)
// {
//     info.anisotropy = u_Anisotropy;
// 
// #ifdef HAS_ANISOTROPY_MAP
//     info.anisotropy *= texture(u_AnisotropySampler, getAnisotropyUV()).r * 2.0 - 1.0;
// #endif
// 
//     return info;
// }
// 
// MaterialInfo getClearCoatInfo(MaterialInfo info, NormalInfo normalInfo)
// {
//     info.clearcoatFactor = u_ClearcoatFactor;
//     info.clearcoatRoughness = u_ClearcoatRoughnessFactor;
//     info.clearcoatF0 = vec3(0.04);
//     info.clearcoatF90 = vec3(clamp(info.clearcoatF0 * 50.0, 0.0, 1.0));
// 
//     #ifdef HAS_CLEARCOAT_TEXTURE_MAP
//         vec4 ccSample = texture(u_ClearcoatSampler, getClearcoatUV());
//         info.clearcoatFactor *= ccSample.r;
//     #endif
// 
//     #ifdef HAS_CLEARCOAT_ROUGHNESS_MAP
//         vec4 ccSampleRough = texture(u_ClearcoatRoughnessSampler, getClearcoatRoughnessUV());
//         info.clearcoatRoughness *= ccSampleRough.g;
//     #endif
// 
//     #ifdef HAS_CLEARCOAT_NORMAL_MAP
//         vec4 ccSampleNor = texture(u_ClearcoatNormalSampler, getClearcoatNormalUV());
//         info.clearcoatNormal = normalize(ccSampleNor.xyz);
//     #else
//         info.clearcoatNormal = normalInfo.ng;
//     #endif
// 
//     info.clearcoatRoughness = clamp(info.clearcoatRoughness, 0.0, 1.0);
// 
//     return info;
// }

void main()
{
    vec4 baseColor = getBaseColor();

#ifdef ALPHAMODE_OPAQUE
    baseColor.a = 1.0;
#endif

#ifdef MATERIAL_UNLIT
    g_finalColor = (vec4(linearTosRGB(baseColor.rgb), baseColor.a));
    return;
#endif

    vec3 v = normalize(v_eye_pos - v_Position);
    NormalInfo normalInfo = getNormalInfo(v);
    vec3 n = normalInfo.n;
    vec3 t = normalInfo.t;
    vec3 b = normalInfo.b;

    float NdotV = clampedDot(n, v);
    float TdotV = clampedDot(t, v);
    float BdotV = clampedDot(b, v);

    MaterialInfo materialInfo;
    materialInfo.baseColor = baseColor.rgb;

#ifdef MATERIAL_IOR
    float ior = u_IOR_and_f0.x;
    float f0_ior = u_IOR_and_f0.y;
#else
    // The default index of refraction of 1.5 yields a dielectric normal incidence reflectance of 0.04.
    float ior = 1.5;
    float f0_ior = 0.04;
#endif

#ifdef MATERIAL_SPECULARGLOSSINESS
    materialInfo = getSpecularGlossinessInfo(materialInfo);
#endif

#ifdef MATERIAL_METALLICROUGHNESS
    materialInfo = getMetallicRoughnessInfo(materialInfo, f0_ior);
#endif

#ifdef MATERIAL_SHEEN
    materialInfo = getSheenInfo(materialInfo);
#endif

#ifdef MATERIAL_SUBSURFACE
    materialInfo = getSubsurfaceInfo(materialInfo);
#endif

#ifdef MATERIAL_THIN_FILM
    materialInfo = getThinFilmInfo(materialInfo);
#endif

#ifdef MATERIAL_CLEARCOAT
    materialInfo = getClearCoatInfo(materialInfo, normalInfo);
#endif

#ifdef MATERIAL_TRANSMISSION
    materialInfo = getTransmissionInfo(materialInfo);
#endif

#ifdef MATERIAL_ANISOTROPY
    materialInfo = getAnisotropyInfo(materialInfo);
#endif

    materialInfo = getThicknessInfo(materialInfo);
    materialInfo = getAbsorptionInfo(materialInfo);

    materialInfo.perceptualRoughness = clamp(materialInfo.perceptualRoughness, 0.0, 1.0);
    materialInfo.metallic = clamp(materialInfo.metallic, 0.0, 1.0);

    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness.
    materialInfo.alphaRoughness = materialInfo.perceptualRoughness * materialInfo.perceptualRoughness;

    // Compute reflectance.
    float reflectance = max(max(materialInfo.f0.r, materialInfo.f0.g), materialInfo.f0.b);

    // Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
    materialInfo.f90 = vec3(clamp(reflectance * 50.0, 0.0, 1.0));

    materialInfo.n = n;

#ifdef MATERIAL_THIN_FILM
    materialInfo.f0 = getThinFilmF0(materialInfo.f0, materialInfo.f90, clampedDot(n, v),
        materialInfo.thinFilmFactor, materialInfo.thinFilmThickness);
#endif

    // LIGHTING
    vec3 f_specular = vec3(0.0);
    vec3 f_diffuse = vec3(0.0);
    vec3 f_emissive = vec3(0.0);
    vec3 f_clearcoat = vec3(0.0);
    vec3 f_sheen = vec3(0.0);
    vec3 f_subsurface = vec3(0.0);
    vec3 f_transmission = vec3(0.0);

    // Calculate lighting contribution from image based lighting source (IBL)
#ifdef USE_IBL
    f_specular += getIBLRadianceGGX(n, v, materialInfo.perceptualRoughness, materialInfo.f0);
    f_diffuse += getIBLRadianceLambertian(n, materialInfo.albedoColor);

    #ifdef MATERIAL_CLEARCOAT
        f_clearcoat += getIBLRadianceGGX(materialInfo.clearcoatNormal, v, materialInfo.clearcoatRoughness, materialInfo.clearcoatF0);
    #endif

    #ifdef MATERIAL_SHEEN
        f_sheen += getIBLRadianceCharlie(n, v, materialInfo.sheenRoughness, materialInfo.sheenColor, materialInfo.sheenIntensity);
    #endif

    #ifdef MATERIAL_SUBSURFACE
        f_subsurface += getIBLRadianceSubsurface(n, v, materialInfo.subsurfaceScale, materialInfo.subsurfaceDistortion, materialInfo.subsurfacePower, materialInfo.subsurfaceColor, materialInfo.subsurfaceThickness);
    #endif

    #ifdef MATERIAL_TRANSMISSION
        f_transmission += getIBLRadianceTransmission(n, v, materialInfo.perceptualRoughness, ior, materialInfo.baseColor);
    #endif
#endif

#ifdef USE_PUNCTUAL
    for (int i = 0; i < LIGHT_COUNT; ++i)
    {
        // Light light = u_Lights[i];

        vec3 pointToLight = -light_direction;
        float rangeAttenuation = 1.0;
        float spotAttenuation = 1.0;

        if(light_type != LightType_Directional)
        {
            pointToLight = light_position - v_Position;
        }

        // // Compute range and spot light attenuation.
        if (light_type != LightType_Directional)
        {
            rangeAttenuation = getRangeAttenuation(light_range, length(pointToLight));
        }
        // if (light_type == LightType_Spot)
        // {
        //     spotAttenuation = getSpotAttenuation(pointToLight, light_direction, light_outerConeCos, light_innerConeCos);
        // }

        vec3 intensity = rangeAttenuation * spotAttenuation * light_intensity * light_color;

        vec3 l = normalize(pointToLight);   // Direction from surface point to light
        vec3 h = normalize(l + v);          // Direction of the vector between l and v, called halfway vector
        float NdotL = clampedDot(n, l);
        float NdotV = clampedDot(n, v);
        float NdotH = clampedDot(n, h);
        float LdotH = clampedDot(l, h);
        float VdotH = clampedDot(v, h);

        if (NdotL > 0.0 || NdotV > 0.0)
        {
            // Calculation of analytical light
            //https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
            f_diffuse += intensity * NdotL *  BRDF_lambertian(materialInfo.f0, materialInfo.f90, materialInfo.albedoColor, VdotH);

            #ifdef MATERIAL_ANISOTROPY
            vec3 h = normalize(l + v);
            float TdotL = dot(t, l);
            float BdotL = dot(b, l);
            float TdotH = dot(t, h);
            float BdotH = dot(b, h);
            f_specular += intensity * NdotL * BRDF_specularAnisotropicGGX(materialInfo.f0, materialInfo.f90, materialInfo.alphaRoughness,
                VdotH, NdotL, NdotV, NdotH,
                BdotV, TdotV, TdotL, BdotL, TdotH, BdotH, materialInfo.anisotropy);
            #else
            f_specular += intensity * NdotL * BRDF_specularGGX(materialInfo.f0, materialInfo.f90, materialInfo.alphaRoughness, VdotH, NdotL, NdotV, NdotH);
            #endif

            #ifdef MATERIAL_SHEEN
                f_sheen += intensity * getPunctualRadianceSheen(materialInfo.sheenColor, materialInfo.sheenIntensity, materialInfo.sheenRoughness,
                    NdotL, NdotV, NdotH);
            #endif

            #ifdef MATERIAL_CLEARCOAT
                f_clearcoat += intensity * getPunctualRadianceClearCoat(materialInfo.clearcoatNormal, v, l,
                    h, VdotH,
                    materialInfo.clearcoatF0, materialInfo.clearcoatF90, materialInfo.clearcoatRoughness);
            #endif
        }

        #ifdef MATERIAL_SUBSURFACE
            f_subsurface += intensity * getPunctualRadianceSubsurface(n, v, l,
                materialInfo.subsurfaceScale, materialInfo.subsurfaceDistortion, materialInfo.subsurfacePower,
                materialInfo.subsurfaceColor, materialInfo.subsurfaceThickness);
        #endif

        #ifdef MATERIAL_TRANSMISSION
            f_transmission += intensity * getPunctualRadianceTransmission(n, v, l, materialInfo.alphaRoughness, ior, materialInfo.f0);
        #endif
    }
#endif // !USE_PUNCTUAL

    f_emissive = u_EmissiveFactor;
#ifdef HAS_EMISSIVE_MAP
    f_emissive *= sRGBToLinear(texture(u_EmissiveSampler, getEmissiveUV())).rgb;
#endif

///
/// Layer blending
///

    float clearcoatFactor = 0.0;
    vec3 clearcoatFresnel = vec3(0.0);

    #ifdef MATERIAL_CLEARCOAT
        clearcoatFactor = materialInfo.clearcoatFactor;
        clearcoatFresnel = F_Schlick(materialInfo.clearcoatF0, materialInfo.clearcoatF90, clampedDot(materialInfo.clearcoatNormal, v));
    #endif

    #ifdef MATERIAL_ABSORPTION
        f_transmission *= transmissionAbsorption(v, n, ior, materialInfo.thickness, materialInfo.absorption);
    #endif

    #ifdef MATERIAL_TRANSMISSION
    vec3 diffuse = mix(f_diffuse, f_transmission, materialInfo.transmission);
    #else
    vec3 diffuse = f_diffuse;
    #endif

    vec3 color = (f_emissive + diffuse + f_specular + f_subsurface + (1.0 - reflectance) * f_sheen) * (1.0 - clearcoatFactor * clearcoatFresnel) + f_clearcoat * clearcoatFactor;

    float ao = 1.0;
    // Apply optional PBR terms for additional (optional) shading
#ifdef HAS_OCCLUSION_MAP
    ao = texture(u_OcclusionSampler,  getOcclusionUV()).r;
    color = mix(color, color * ao, u_OcclusionStrength);
#endif

#ifndef DEBUG_OUTPUT // no debug

#ifdef ALPHAMODE_MASK
    // Late discard to avaoid samplig artifacts. See https://github.com/KhronosGroup/glTF-Sample-Viewer/issues/267
    if(baseColor.a < u_AlphaCutoff)
    {
        discard;
    }
    baseColor.a = 1.0;
#endif

    // regular shading
    // g_finalColor = vec4(toneMap(color), baseColor.a);
    g_finalColor = vec4(color, baseColor.a);
    // g_finalColor = vec4(1,0,0,1);

#else // debug output

    #ifdef DEBUG_METALLIC
        g_finalColor.rgb = vec3(materialInfo.metallic);
    #endif

    #ifdef DEBUG_ROUGHNESS
        g_finalColor.rgb = vec3(materialInfo.perceptualRoughness);
    #endif

    #ifdef DEBUG_NORMAL
        #ifdef HAS_NORMAL_MAP
            g_finalColor.rgb = texture(u_NormalSampler, getNormalUV()).rgb;
        #else
            g_finalColor.rgb = vec3(0.5, 0.5, 1.0);
        #endif
    #endif

    #ifdef DEBUG_TANGENT
        g_finalColor.rgb = t * 0.5 + vec3(0.5);
    #endif

    #ifdef DEBUG_BITANGENT
        g_finalColor.rgb = b * 0.5 + vec3(0.5);
    #endif

    #ifdef DEBUG_BASECOLOR
        g_finalColor.rgb = linearTosRGB(materialInfo.baseColor);
    #endif

    #ifdef DEBUG_OCCLUSION
        g_finalColor.rgb = vec3(ao);
    #endif

    #ifdef DEBUG_F0
        g_finalColor.rgb = materialInfo.f0;
    #endif

    #ifdef DEBUG_FEMISSIVE
        g_finalColor.rgb = f_emissive;
    #endif

    #ifdef DEBUG_FSPECULAR
        g_finalColor.rgb = f_specular;
    #endif

    #ifdef DEBUG_FDIFFUSE
        g_finalColor.rgb = f_diffuse;
    #endif

    #ifdef DEBUG_THICKNESS
        g_finalColor.rgb = vec3(materialInfo.thickness);
    #endif

    #ifdef DEBUG_FCLEARCOAT
        g_finalColor.rgb = f_clearcoat;
    #endif

    #ifdef DEBUG_FSHEEN
        g_finalColor.rgb = f_sheen;
    #endif

    #ifdef DEBUG_ALPHA
        g_finalColor.rgb = vec3(baseColor.a);
    #endif

    #ifdef DEBUG_FSUBSURFACE
        g_finalColor.rgb = f_subsurface;
    #endif

    #ifdef DEBUG_FTRANSMISSION
        g_finalColor.rgb = linearTosRGB(f_transmission);
    #endif

    g_finalColor.a = 1.0;

#endif // !DEBUG_OUTPUT
}

@end


@program offscreen_pbr vs_pbr fs_pbr

