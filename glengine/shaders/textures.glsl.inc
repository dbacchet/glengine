// textures
// General Material
uniform sampler2D u_NormalSampler;
uniform sampler2D u_EmissiveSampler;
uniform sampler2D u_OcclusionSampler;
// Metallic Roughness Material
uniform sampler2D u_BaseColorSampler;
uniform sampler2D u_MetallicRoughnessSampler;
// Specular Glossiness Material
uniform sampler2D u_DiffuseSampler;
uniform sampler2D u_SpecularGlossinessSampler;
// IBL
uniform samplerCube u_LambertianEnvSampler;
uniform samplerCube u_GGXEnvSampler;
uniform sampler2D u_GGXLUT;
uniform samplerCube u_CharlieEnvSampler;
uniform sampler2D u_CharlieLUT;
//clearcoat
uniform sampler2D u_ClearcoatSampler;
uniform sampler2D u_ClearcoatRoughnessSampler;
uniform sampler2D u_ClearcoatNormalSampler;
//sheen
uniform sampler2D u_SheenColorIntensitySampler;
//specular
uniform sampler2D u_MetallicRoughnessSpecularSampler;
//subsurface
uniform sampler2D u_SubsurfaceColorSampler;
uniform sampler2D u_SubsurfaceThicknessSampler;
//thin film
uniform sampler2D u_ThinFilmLUT;
uniform sampler2D u_ThinFilmSampler;
uniform sampler2D u_ThinFilmThicknessSampler;
// Thickness:
uniform sampler2D u_ThicknessSampler;
// Anisotropy:
uniform sampler2D u_AnisotropySampler;
uniform sampler2D u_AnisotropyDirectionSampler;

// texture parameters
uniform TextureParams {
    // General Material
    float u_NormalScale;
    // mat3 u_NormalUVTransform;
    vec3 u_EmissiveFactor;
    // mat3 u_EmissiveUVTransform;
    float u_OcclusionStrength;
    // mat3 u_OcclusionUVTransform;
    // Metallic Roughness Material
    // mat3 u_BaseColorUVTransform;
    // mat3 u_MetallicRoughnessUVTransform;
    // Specular Glossiness Material
    // mat3 u_DiffuseUVTransform;
    // mat3 u_SpecularGlossinessUVTransform;
    // IBL
    float u_MipCount;
    //clearcoat
    // mat3 u_ClearcoatUVTransform;
    // mat3 u_ClearcoatRoughnessUVTransform;
    // mat3 u_ClearcoatNormalUVTransform;
    //sheen
    // mat3 u_SheenColorIntensityUVTransform;
    //specular
    // mat3 u_MetallicRougnessSpecularUVTransform;
    //subsurface
    // mat3 u_SubsurfaceColorUVTransform;
    // mat3 u_SubsurfaceThicknessUVTransform;
    //thin film
    // mat3 u_ThinFilmUVTransform;
    // mat3 u_ThinFilmThicknessUVTransform;
    // Thickness:
    // mat3 u_ThicknessUVTransform;
    // Anisotropy:
    // mat3 u_AnisotropyUVTransform;
    // mat3 u_AnisotropyDirectionUVTransform;
};

// int params can not be uniforms in sokol; hardcode
// General Material
int u_NormalUVSet = 0;
int u_EmissiveUVSet = 0;
int u_OcclusionUVSet = 0;
// Metallic Roughness Material
int u_BaseColorUVSet = 0;
int u_MetallicRoughnessUVSet = 0;
// Specular Glossiness Material
int u_DiffuseUVSet = 0;
int u_SpecularGlossinessUVSet = 0;
//clearcoat
int u_ClearcoatUVSet = 0;
int u_ClearcoatRoughnessUVSet = 0;
int u_ClearcoatNormalUVSet = 0;
//sheen
int u_SheenColorIntensityUVSet = 0;
//specular
int u_MetallicRougnessSpecularTextureUVSet = 0;
//subsurface
int u_SubsurfaceColorUVSet = 0;
int u_SubsurfaceThicknessUVSet = 0;
//thin film
int u_ThinFilmUVSet = 0;
int u_ThinFilmThicknessUVSet = 0;
// Thickness:
int u_ThicknessUVSet = 0;
// Anisotropy:
int u_AnisotropyUVSet = 0;
int u_AnisotropyDirectionUVSet = 0;


vec2 getNormalUV()
{
    vec3 uv = vec3(u_NormalUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_NORMAL_UV_TRANSFORM
    uv *= u_NormalUVTransform;
    #endif

    return uv.xy;
}

vec2 getEmissiveUV()
{
    vec3 uv = vec3(u_EmissiveUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_EMISSIVE_UV_TRANSFORM
    uv *= u_EmissiveUVTransform;
    #endif

    return uv.xy;
}

vec2 getOcclusionUV()
{
    vec3 uv = vec3(u_OcclusionUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_OCCLUSION_UV_TRANSFORM
    uv *= u_OcclusionUVTransform;
    #endif

    return uv.xy;
}

vec2 getBaseColorUV()
{
    vec3 uv = vec3(u_BaseColorUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_BASECOLOR_UV_TRANSFORM
    uv *= u_BaseColorUVTransform;
    #endif

    return uv.xy;
}

vec2 getMetallicRoughnessUV()
{
    vec3 uv = vec3(u_MetallicRoughnessUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_METALLICROUGHNESS_UV_TRANSFORM
    uv *= u_MetallicRoughnessUVTransform;
    #endif

    return uv.xy;
}

vec2 getSpecularGlossinessUV()
{
    vec3 uv = vec3(u_SpecularGlossinessUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_SPECULARGLOSSINESS_UV_TRANSFORM
    uv *= u_SpecularGlossinessUVTransform;
    #endif

    return uv.xy;
}

vec2 getDiffuseUV()
{
    vec3 uv = vec3(u_DiffuseUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_DIFFUSE_UV_TRANSFORM
    uv *= u_DiffuseUVTransform;
    #endif

    return uv.xy;
}

vec2 getClearcoatUV()
{
    vec3 uv = vec3(u_ClearcoatUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);
    #ifdef HAS_CLEARCOAT_UV_TRANSFORM
    uv *= u_ClearcoatUVTransform;
    #endif
    return uv.xy;
}

vec2 getClearcoatRoughnessUV()
{
    vec3 uv = vec3(u_ClearcoatRoughnessUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);
    #ifdef HAS_CLEARCOATROUGHNESS_UV_TRANSFORM
    uv *= u_ClearcoatRoughnessUVTransform;
    #endif
    return uv.xy;
}

vec2 getClearcoatNormalUV()
{
    vec3 uv = vec3(u_ClearcoatNormalUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);
    #ifdef HAS_CLEARCOATNORMAL_UV_TRANSFORM
    uv *= u_ClearcoatNormalUVTransform;
    #endif
    return uv.xy;
}

vec2 getSheenUV()
{
    vec3 uv = vec3(u_SheenColorIntensityUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);
    #ifdef HAS_SHEENCOLORINTENSITY_UV_TRANSFORM
    uv *= u_SheenUVTransform;
    #endif
    return uv.xy;
}

vec2 getMetallicRoughnessSpecularUV()
{
    vec3 uv = vec3(u_MetallicRougnessSpecularTextureUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);
    #ifdef HAS_METALLICROUGHNESSSPECULAR_UV_TRANSFORM
    uv *= u_MetallicRougnessSpecularUVTransform;
    #endif
    return uv.xy;
}

vec2 getSubsurfaceColorUV()
{
    vec3 uv = vec3(u_SubsurfaceColorUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);
    #ifdef HAS_SUBSURFACECOLOR_UV_TRANSFORM
    uv *= u_SubsurfaceColorUVTransform;
    #endif
    return uv.xy;
}

vec2 getSubsurfaceThicknessUV()
{
    vec3 uv = vec3(u_SubsurfaceThicknessUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);
    #ifdef HAS_SUBSURFACETHICKNESS_UV_TRANSFORM
    uv *= u_SubsurfaceThicknessUVTransform;
    #endif
    return uv.xy;
}

vec2 getThinFilmUV()
{
    vec3 uv = vec3(u_ThinFilmUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_THIN_FILM_UV_TRANSFORM
    uv *= u_ThinFilmUVTransform;
    #endif

    return uv.xy;
}

vec2 getThinFilmThicknessUV()
{
    vec3 uv = vec3(u_ThinFilmThicknessUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_THIN_FILM_THICKNESS_UV_TRANSFORM
    uv *= u_ThinFilmThicknessUVTransform;
    #endif

    return uv.xy;
}

vec2 getThicknessUV()
{
    vec3 uv = vec3(u_ThicknessUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_THICKNESS_UV_TRANSFORM
    uv *= u_ThicknessUVTransform;
    #endif

    return uv.xy;
}

vec2 getAnisotropyUV()
{
    vec3 uv = vec3(u_AnisotropyUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_ANISOTROPY_UV_TRANSFORM
    uv *= u_AnisotropyUVTransform;
    #endif

    return uv.xy;
}

vec2 getAnisotropyDirectionUV()
{
    vec3 uv = vec3(u_AnisotropyDirectionUVSet < 1 ? v_UVCoord1 : v_UVCoord2, 1.0);

    #ifdef HAS_ANISOTROPY_DIRECTION_UV_TRANSFORM
    uv *= u_AnisotropyDirectionUVTransform;
    #endif

    return uv.xy;
}
