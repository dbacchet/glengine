#include "gl_effect_blur.h"

#include "gl_engine.h"
#include "generated/shaders/ssao_blur.glsl.h"

#include "sokol_gfx.h"

#include <random>

namespace glengine {

bool EffectBlur::init(GLEngine &eng, sg_primitive_type primitive, sg_index_type idx_type) {
    _eng = &eng;
    ResourceManager &rm = eng.resource_manager();
    sg_shader shader = rm.get_or_create_shader(*ssao_blur_shader_desc());

    sg_pipeline_desc pip_desc = {0};
    pip_desc.layout.attrs[ATTR_vs_blur_pos].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.shader = shader;
    pip_desc.primitive_type = primitive;
    pip_desc.index_type = idx_type;
    pip_desc.blend = {.color_attachment_count = 1, .depth_format = SG_PIXELFORMAT_NONE};
    pip_desc.rasterizer = {.cull_mode = SG_CULLMODE_NONE, .face_winding = SG_FACEWINDING_CCW};
    pip_desc.label = "ssao blur";
    pip = rm.get_or_create_pipeline(pip_desc);
    // quad vertices
    float quad_vertices[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    bind.vertex_buffers[0] = sg_make_buffer(
        (sg_buffer_desc){.size = sizeof(quad_vertices), .content = quad_vertices, .label = "quad vertices"});

    initialized = true;
    return initialized;
}

void EffectBlur::update_bindings() {
    bind.fs_images[SLOT_tex] = tex;
}

void EffectBlur::apply_uniforms() {
    // ssao_params_t ssao_params{
    //     .u_view = _eng->_camera.inverse_transform(),
    //     .u_projection = _eng->_camera.projection(),
    //     .u_inv_projection = math::inverse(_eng->_camera.projection()),
    //     .radius = radius,
    //     .noise_scale = math::Vector4f(_eng->_context.window_state.window_size.x / 4,
    //                                   _eng->_context.window_state.window_size.y / 4, 0, 0),
    //     .bias = bias,
    // };
    // sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_ssao_params, &ssao_params, sizeof(ssao_params));
}

} // namespace glengine
