#include "gl_material_flat.h"

#include "gl_engine.h"
#include "generated/shaders/multipass-flat.glsl.h"

#include "sokol_gfx.h"

namespace glengine {

bool MaterialFlat::init(GLEngine &eng, sg_primitive_type primitive, sg_index_type idx_type) {
    ResourceManager &rm = eng.resource_manager();
    sg_shader offscreen_vertexcolor = rm.get_or_create_shader(*offscreen_flat_shader_desc(sg_query_backend()));

    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? eng._config.msaa_samples : 1;
    sg_pipeline_desc pip_desc = {0};
    pip_desc.layout.buffers[0].stride = sizeof(Vertex);
    pip_desc.layout.attrs[ATTR_vs_flat_vertex_pos].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.shader = offscreen_vertexcolor, pip_desc.primitive_type = primitive, pip_desc.index_type = idx_type;
    pip_desc.depth = {.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL, .compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true};
    if (eng._config.use_mrt) {
        pip_desc.color_count = 3;
    } else { // only 1 color attachment
        pip_desc.color_count = 1;
    }
    pip_desc.cull_mode = SG_CULLMODE_NONE;
    pip_desc.face_winding = SG_FACEWINDING_CCW;
    pip_desc.sample_count = offscreen_sample_count;
    pip_desc.label = "flat pipeline";
    pip = rm.get_or_create_pipeline(pip_desc);
    return true;
}

void MaterialFlat::update_bindings(sg_bindings &bind) {
    // nothing to be done here
}

void MaterialFlat::apply_uniforms(const common_uniform_params_t &params) {
    vs_params_t vs_params{.model = params.model, .view = params.view, .projection = params.projection};
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));
    fs_params_t fs_params{.color = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f}};
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params, SG_RANGE(fs_params));
}


// ///////////// //
// flat-textured //
// ///////////// //

bool MaterialFlatTextured::init(GLEngine &eng, sg_primitive_type primitive, sg_index_type idx_type) {
    ResourceManager &rm = eng.resource_manager();
    sg_shader offscreen_vertexcolor = rm.get_or_create_shader(*offscreen_flat_textured_shader_desc(sg_query_backend()));

    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? eng._config.msaa_samples : 1;
    sg_pipeline_desc pip_desc = {0};
    pip_desc.layout.buffers[0].stride = sizeof(Vertex);
    pip_desc.layout.attrs[ATTR_vs_flat_textured_vertex_pos].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_flat_textured_vertex_color].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_desc.layout.attrs[ATTR_vs_flat_textured_vertex_normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_flat_textured_vertex_texcoord].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.shader = offscreen_vertexcolor, pip_desc.primitive_type = primitive, pip_desc.index_type = idx_type;
    pip_desc.depth = {.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL, .compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true};
    if (eng._config.use_mrt) {
        pip_desc.color_count = 3;
    } else { // only 1 color attachment
        pip_desc.color_count = 1;
    }
    pip_desc.cull_mode = SG_CULLMODE_NONE;
    pip_desc.face_winding = SG_FACEWINDING_CCW;
    pip_desc.sample_count = offscreen_sample_count;
    pip_desc.label = "flat textured pipeline";
    pip = rm.get_or_create_pipeline(pip_desc);
    return true;
}

void MaterialFlatTextured::update_bindings(sg_bindings &bind) {
    bind.fs_images[SLOT_tex_diffuse] = tex_diffuse;
}

void MaterialFlatTextured::apply_uniforms(const common_uniform_params_t &params) {
    vs_params_t vs_params{.model = params.model, .view = params.view, .projection = params.projection};
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));
    fs_params_t fs_params{.color = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f}};
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params, SG_RANGE(fs_params));
}
} // namespace glengine
