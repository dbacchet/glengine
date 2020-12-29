#include "gl_material_vertexcolor.h"

#include "gl_materialsystem.h"
#include "shaders/generated/multipass-vertexcolor.glsl.h"

#include "sokol_gfx.h"

namespace glengine {

bool MaterialVertexColor::init(sg_primitive_type primitive, sg_index_type idx_type) {
    auto &ms = glengine::MaterialSystem::get();
    sg_shader offscreen_vertexcolor = ms.get_or_create_shader(*offscreen_vertexcolor_shader_desc());

    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? 4 : 1;
    sg_pipeline_desc pip_desc = {
        .layout = {.buffers[0].stride = sizeof(Vertex),
                   .attrs = {[ATTR_vs_vertexcolor_vertex_pos].format = SG_VERTEXFORMAT_FLOAT3,
                             [ATTR_vs_vertexcolor_vertex_col].format = SG_VERTEXFORMAT_UBYTE4N}},
        .shader = offscreen_vertexcolor,
        .primitive_type = primitive,
        .index_type = idx_type,
        .depth_stencil = {.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL, .depth_write_enabled = true},
        .blend = {.color_attachment_count = 1, .depth_format = SG_PIXELFORMAT_DEPTH},
        .rasterizer = {.cull_mode = SG_CULLMODE_NONE, .sample_count = offscreen_sample_count},
        .label = "vertexcolor pipeline"};
    pip = ms.get_or_create_pipeline(pip_desc);
    return true;
}

void MaterialVertexColor::update_bindings(sg_bindings &bind) {
    // nothing to be done here
}

} // namespace glengine
