#include "gl_material_flat_textured.h"

#include "gl_resource_manager.h"
#include "shaders/generated/multipass-flat-textured.glsl.h"

#include "sokol_gfx.h"

namespace glengine {

bool MaterialFlatTextured::init(ResourceManager &rm, sg_primitive_type primitive, sg_index_type idx_type) {
    // auto &rm = glengine::ResourceManager::get();
    sg_shader offscreen_vertexcolor = rm.get_or_create_shader(*offscreen_flat_textured_shader_desc());

    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? 4 : 1;
    sg_pipeline_desc pip_desc = {
        .layout = {.buffers[0].stride = sizeof(Vertex),
                   .attrs =
                       {
                           [ATTR_vs_flat_textured_vertex_pos].format = SG_VERTEXFORMAT_FLOAT3,
                           [ATTR_vs_flat_textured_vertex_color].format = SG_VERTEXFORMAT_BYTE4N,
                           [ATTR_vs_flat_textured_vertex_normal].format = SG_VERTEXFORMAT_FLOAT3,
                           [ATTR_vs_flat_textured_vertex_texcoord].format = SG_VERTEXFORMAT_FLOAT2,
                       }},
        .shader = offscreen_vertexcolor,
        .primitive_type = primitive,
        .index_type = idx_type,
        .depth_stencil = {.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL, .depth_write_enabled = true},
        .blend = {.color_attachment_count = 1, .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL},
        .rasterizer = {.cull_mode = SG_CULLMODE_NONE,
                       .face_winding = SG_FACEWINDING_CCW,
                       .sample_count = offscreen_sample_count},
        .label = "flat pipeline"};
    pip = rm.get_or_create_pipeline(pip_desc);
    return true;
}

void MaterialFlatTextured::update_bindings(sg_bindings &bind) {
    bind.fs_images[SLOT_tex_diffuse] = tex_diffuse;
}

void MaterialFlatTextured::apply_uniforms(const common_uniform_params_t &params) {
    vs_params_t vs_params{.model = params.model, .view = params.view, .projection = params.projection};
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    fs_params_t fs_params{.color = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f}};
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params, &fs_params, sizeof(fs_params));
}

} // namespace glengine
