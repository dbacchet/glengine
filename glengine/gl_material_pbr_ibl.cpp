#include "gl_material_pbr_ibl.h"

#include "gl_resource_manager.h"
#include "shaders/generated/pbr_ibl.glsl.h"

#include "sokol_gfx.h"

namespace {

struct Placeholders {
    sg_image white;
    sg_image normal;
    sg_image black;
    sg_image lut;
    sg_image env_diffuse;
    sg_image env_specular;
};

Placeholders placeholders;
bool have_placeholders = false;

void create_placeholder_textures() {
    // create placeholder textures
    uint32_t pixels[64];
    sg_image_desc img = {
        .width = 8,
        .height = 8,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
    };
    img.content.subimage[0][0] = {.ptr = pixels, .size = sizeof(pixels)};
    // white
    for (int i = 0; i < 64; i++) {
        pixels[i] = 0xFFFFFFFF;
    }
    placeholders.white = sg_make_image(img);
    // black
    for (int i = 0; i < 64; i++) {
        pixels[i] = 0xFF000000;
    }
    placeholders.black = sg_make_image(img);
    // normal
    for (int i = 0; i < 64; i++) {
        pixels[i] = 0xFFFF8080;
    }
    placeholders.normal = sg_make_image(img);
}

} // namespace

namespace glengine {

bool MaterialPBRIBL::init(ResourceManager &rm, sg_primitive_type primitive, sg_index_type idx_type) {
    sg_shader offscreen_vertexcolor = rm.get_or_create_shader(*offscreen_pbr_ibl_shader_desc());

    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? 4 : 1;
    sg_pipeline_desc pip_desc = {0};
    pip_desc.layout.buffers[0].stride = sizeof(Vertex);
    pip_desc.layout.attrs[ATTR_vs_pbr_ibl_a_Position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_pbr_ibl_a_Color].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_desc.layout.attrs[ATTR_vs_pbr_ibl_a_Normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_pbr_ibl_a_UV1].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.layout.attrs[ATTR_vs_pbr_ibl_a_Tangent].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.shader = offscreen_vertexcolor, pip_desc.primitive_type = primitive, pip_desc.index_type = idx_type;
    pip_desc.depth_stencil = {.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL, .depth_write_enabled = true};
    pip_desc.blend = {.color_attachment_count = 1, .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL};
    pip_desc.rasterizer = {
        .cull_mode = SG_CULLMODE_NONE, .face_winding = SG_FACEWINDING_CCW, .sample_count = offscreen_sample_count};
    pip_desc.label = "PBR pipeline";
    pip = rm.get_or_create_pipeline(pip_desc);
    // placeholder textures
    if (!have_placeholders) {
        create_placeholder_textures();
        placeholders.lut = rm.get_or_create_image("../resources/textures/lut_ggx.png");
        placeholders.env_diffuse = rm.get_or_create_image("../resources/textures/doge2-diffuse-RGBM.png");
        placeholders.env_specular = rm.get_or_create_image("../resources/textures/doge2-specular-RGBM.png");
        have_placeholders = true;
    }

    tex_diffuse = placeholders.white;
    tex_metallic_roughness = placeholders.white;
    tex_normal = placeholders.normal;
    tex_occlusion = placeholders.white;
    tex_emissive = placeholders.black;
    color = {255, 255, 255, 255};
    return true;
}

void MaterialPBRIBL::update_bindings(sg_bindings &bind) {
    bind.fs_images[SLOT_u_BaseColorSampler] = tex_diffuse;
    bind.fs_images[SLOT_u_MetallicRoughnessSampler] = tex_metallic_roughness;
    bind.fs_images[SLOT_u_NormalSampler] = tex_normal;
    bind.fs_images[SLOT_u_OcclusionSampler] = tex_occlusion;
    bind.fs_images[SLOT_u_EmissiveSampler] = tex_emissive;

    bind.fs_images[SLOT_u_GGXLUT] = placeholders.lut;
    bind.fs_images[SLOT_u_LambertianEnvSampler] = placeholders.env_diffuse;
    bind.fs_images[SLOT_u_GGXEnvSampler] = placeholders.env_specular;
}

void MaterialPBRIBL::apply_uniforms(const common_uniform_params_t &params) {
    vs_params_t vs_params{.model = params.model, .view = params.view, .projection = params.projection};
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    fs_params_t mparams{
        .u_MetallicFactor = metallic_factor,
        .u_RoughnessFactor = roughness_factor,
        .u_BaseColorFactor = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f},
        .u_Exposure = 1.0f,
    };
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params, &mparams, sizeof(mparams));
    TextureParams_t tparams{
        .u_NormalScale = 1.0f,
        .u_EmissiveFactor = emissive_factor,
        .u_OcclusionStrength = 1.0f,
        .u_MipCount = 1,
    };
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_TextureParams, &tparams, sizeof(tparams));
}






bool MaterialPBRIBLAlternative::init(ResourceManager &rm, sg_primitive_type primitive, sg_index_type idx_type) {
    sg_shader offscreen_vertexcolor = rm.get_or_create_shader(*offscreen_pbr_alternative_ibl_shader_desc());

    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? 4 : 1;
    sg_pipeline_desc pip_desc = {0};
    pip_desc.layout.buffers[0].stride = sizeof(Vertex);
    pip_desc.layout.attrs[ATTR_vs_pbr_alternative_a_Position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_pbr_alternative_a_Color].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_desc.layout.attrs[ATTR_vs_pbr_alternative_a_Normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_pbr_alternative_a_UV1].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.shader = offscreen_vertexcolor, pip_desc.primitive_type = primitive, pip_desc.index_type = idx_type;
    pip_desc.depth_stencil = {.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL, .depth_write_enabled = true};
    pip_desc.blend = {.color_attachment_count = 1, .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL};
    pip_desc.rasterizer = {
        .cull_mode = SG_CULLMODE_NONE, .face_winding = SG_FACEWINDING_CCW, .sample_count = offscreen_sample_count};
    pip_desc.label = "PBR pipeline";
    pip = rm.get_or_create_pipeline(pip_desc);
    // placeholder textures
    if (!have_placeholders) {
        create_placeholder_textures();
        placeholders.lut = rm.get_or_create_image("../resources/textures/lut_ggx.png");
        placeholders.env_diffuse = rm.get_or_create_image("../resources/textures/doge2-diffuse-RGBM.png");
        placeholders.env_specular = rm.get_or_create_image("../resources/textures/doge2-specular-RGBM.png");
        have_placeholders = true;
    }

    tex_diffuse = placeholders.white;
    tex_metallic_roughness = placeholders.white;
    tex_normal = placeholders.normal;
    tex_occlusion = placeholders.white;
    tex_emissive = placeholders.black;
    color = {255, 255, 255, 255};
    return true;
}

void MaterialPBRIBLAlternative::update_bindings(sg_bindings &bind) {
    bind.fs_images[SLOT_tBaseColor] = tex_diffuse;
    bind.fs_images[SLOT_tRMO] = tex_metallic_roughness;
    bind.fs_images[SLOT_tNormal] = tex_normal;
    // bind.fs_images[SLOT_u_OcclusionSampler] = tex_occlusion;
    bind.fs_images[SLOT_tEmissive] = tex_emissive;
    bind.fs_images[SLOT_tLUT] = placeholders.lut;
    bind.fs_images[SLOT_tEnvDiffuse] = placeholders.env_diffuse;
    bind.fs_images[SLOT_tEnvSpecular] = placeholders.env_specular;
}

void MaterialPBRIBLAlternative::apply_uniforms(const common_uniform_params_t &params) {
    vs_params_t vs_params{.model = params.model, .view = params.view, .projection = params.projection};
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    fs_params_alternative_t fparams = {
        .viewMatrix = params.view,
        .uBaseColor = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f},
        .uAlpha = 1.0f,
        .uMetallic = metallic_factor,
        .uRoughness = roughness_factor,
        .uOcclusion = 1.0f,
        .uNormalScale = 1.0f,
        .uNormalUVScale = 1.0f,
        .uEmissive = 1.0f,
        .uEnvSpecular = 1.0f,
        .uLightDirection = {-0.7398999929428101, 0.19830000400543213, -0.642799973487854},
        .uLightColor = {0.001f, 0.001f, 0.001f},
    };
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params_alternative, &fparams, sizeof(fparams));
}

} // namespace glengine
