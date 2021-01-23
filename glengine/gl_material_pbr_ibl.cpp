#include "gl_material_pbr_ibl.h"

#include "gl_engine.h"
#include "generated/shaders/pbr_ibl.glsl.h"

#include "sokol_gfx.h"

namespace {

/// \todo remove this hidden global struct
struct Placeholders {
    sg_image lut;
    sg_image env_diffuse;
    sg_image env_specular;
};

Placeholders placeholders;
bool have_placeholders = false;

} // namespace

namespace glengine {

bool MaterialPBRIBL::init(GLEngine &eng, sg_primitive_type primitive, sg_index_type idx_type) {
    ResourceManager &rm = eng.resource_manager();
    sg_shader offscreen_vertexcolor = rm.get_or_create_shader(*offscreen_pbr_ibl_shader_desc());

    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? eng._config.msaa_samples : 1;
    const int color_attachment_count = eng._config.use_mrt ? 3 : 1;
    sg_pipeline_desc pip_desc = {0};
    pip_desc.layout.buffers[0].stride = sizeof(Vertex);
    pip_desc.layout.attrs[ATTR_vs_pbr_ibl_a_Position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_pbr_ibl_a_Color].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_desc.layout.attrs[ATTR_vs_pbr_ibl_a_Normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_pbr_ibl_a_UV1].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.layout.attrs[ATTR_vs_pbr_ibl_a_Tangent].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.shader = offscreen_vertexcolor, pip_desc.primitive_type = primitive, pip_desc.index_type = idx_type;
    pip_desc.depth_stencil = {.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL, .depth_write_enabled = true};
    pip_desc.blend = {.color_attachment_count = color_attachment_count, .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL};
    pip_desc.rasterizer = {
        .cull_mode = SG_CULLMODE_NONE, .face_winding = SG_FACEWINDING_CCW, .sample_count = offscreen_sample_count};
    pip_desc.label = "PBR pipeline";
    pip = rm.get_or_create_pipeline(pip_desc);
    // placeholder textures
    if (!have_placeholders) {
        placeholders.lut = rm.get_or_create_image("../resources/textures/lut_ggx.png");
        placeholders.env_diffuse = rm.get_or_create_image("../resources/textures/doge2-diffuse-RGBM.png");
        placeholders.env_specular = rm.get_or_create_image("../resources/textures/doge2-specular-RGBM.png");
        have_placeholders = true;
    }

    tex_diffuse = rm.default_image(ResourceManager::White);
    tex_metallic_roughness = rm.default_image(ResourceManager::White);
    tex_normal = rm.default_image(ResourceManager::Normal);
    tex_occlusion = rm.default_image(ResourceManager::White);
    tex_emissive = rm.default_image(ResourceManager::Black);
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

} // namespace glengine
