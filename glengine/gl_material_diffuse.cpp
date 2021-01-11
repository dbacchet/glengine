#include "gl_material_diffuse.h"

#include "gl_resource_manager.h"
#include "shaders/generated/multipass-diffuse.glsl.h"

#include "sokol_gfx.h"

namespace glengine {

bool MaterialDiffuse::init(ResourceManager &rm, sg_primitive_type primitive, sg_index_type idx_type) {
    // auto &rm = glengine::ResourceManager::get();
    sg_shader offscreen_vertexcolor = rm.get_or_create_shader(*offscreen_diffuse_shader_desc());

    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? 4 : 1;
    sg_pipeline_desc pip_desc = {0};
    pip_desc.layout.buffers[0].stride = sizeof(Vertex);
    pip_desc.layout.attrs[ATTR_vs_diffuse_vertex_pos].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_diffuse_vertex_color].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_desc.layout.attrs[ATTR_vs_diffuse_vertex_normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.shader = offscreen_vertexcolor, pip_desc.primitive_type = primitive, pip_desc.index_type = idx_type;
    pip_desc.depth_stencil = {.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL, .depth_write_enabled = true};
    pip_desc.blend = {.color_attachment_count = 1, .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL};
    pip_desc.rasterizer = {
        .cull_mode = SG_CULLMODE_NONE, .face_winding = SG_FACEWINDING_CCW, .sample_count = offscreen_sample_count};
    pip_desc.label = "diffuse pipeline";
    pip = rm.get_or_create_pipeline(pip_desc);
    return true;
}

void MaterialDiffuse::update_bindings(sg_bindings &bind) {
    // nothing to be done here
}

void MaterialDiffuse::apply_uniforms(const common_uniform_params_t &params) {
    vs_params_t vs_params{.model = params.model, .view = params.view, .projection = params.projection};
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    fs_params_t fs_params{.color = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f}};
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params, &fs_params, sizeof(fs_params));
}

} // namespace glengine
