#include "gl_material_pbr.h"

#include "gl_resource_manager.h"
#include "shaders/generated/multipass-pbr.glsl.h"

#include "sokol_gfx.h"

namespace {

struct Placeholders {
    sg_image white;
    sg_image normal;
    sg_image black;
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
        pixels[i] = 0xFF0000FF;
    }
    placeholders.normal = sg_make_image(img);
}

} // namespace

namespace glengine {

bool MaterialPBR::init(ResourceManager &rm, sg_primitive_type primitive, sg_index_type idx_type) {
    sg_shader offscreen_vertexcolor = rm.get_or_create_shader(*offscreen_pbr_shader_desc());

    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? 4 : 1;
    sg_pipeline_desc pip_desc = {0};
    pip_desc.layout.buffers[0].stride = sizeof(Vertex);
    pip_desc.layout.attrs[ATTR_vs_pbr_vertex_pos].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_pbr_vertex_color].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_desc.layout.attrs[ATTR_vs_pbr_vertex_normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_pbr_vertex_texcoord].format = SG_VERTEXFORMAT_FLOAT2;
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
    }

    tex_diffuse = placeholders.white;
    tex_metallic_roughness = placeholders.white;
    tex_normal = placeholders.normal;
    tex_occlusion = placeholders.white;
    tex_emissive = placeholders.black;
    color = {255,255,255,255};
    return true;
}

void MaterialPBR::update_bindings(sg_bindings &bind) {
    bind.fs_images[SLOT_base_color_texture] = tex_diffuse;
    bind.fs_images[SLOT_metallic_roughness_texture] = tex_metallic_roughness;
    bind.fs_images[SLOT_normal_texture] = tex_normal;
    bind.fs_images[SLOT_occlusion_texture] = tex_occlusion;
    bind.fs_images[SLOT_emissive_texture] = tex_emissive;
}

void MaterialPBR::apply_uniforms(const common_uniform_params_t &params) {
    vs_params_t vs_params{.model = params.model, .view = params.view, .projection = params.projection};
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    light_params_t lparams{.light_pos = {10.0f, 10.0f, 10.0f},
                           .light_range = 200.0f,
                           .light_color = {1.0f, 1.0f, 1.0f},
                           .light_intensity = 700.0f};
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_light_params, &lparams, sizeof(lparams));
    metallic_params_t mparams{
        .base_color_factor = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f},
        .emissive_factor = {0.5f, 0.5f, 0.5f},
        .metallic_factor = 0.5f,
        .roughness_factor = 0.3f,
    };
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_metallic_params, &mparams, sizeof(mparams));
}

} // namespace glengine
