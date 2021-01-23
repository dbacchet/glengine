#include "gl_effect_ssao.h"
#include "gl_context.h"
#include "gl_engine.h"
#include "generated/shaders/ssao.glsl.h"

#include "sokol_gfx.h"

#include <random>

namespace {

struct Placeholders {
    sg_image random;
};

Placeholders placeholders;
bool have_placeholders = false;

void create_placeholder_textures() {
    // create placeholder textures
    math::Vector4f pixels[16];
    sg_image_desc img = {
        .width = 4,
        .height = 4,
        .pixel_format = SG_PIXELFORMAT_RGBA32F,
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
    };
    img.content.subimage[0][0] = {.ptr = pixels, .size = sizeof(pixels)};
    // ssao noise texture
    std::mt19937 gen(12345678);
    std::uniform_real_distribution<float> random_float(-1.0, 1.0); // generates random floats between -1.0 and 1.0
    for (unsigned int i = 0; i < 16; i++) {
        pixels[i] = {random_float(gen), random_float(gen), 0.0f, 0.0f}; // rotate around z-axis (in tangent space)
    }
    placeholders.random = sg_make_image(img);
}

} // namespace

namespace glengine {

bool EffectSSAO::init(GLEngine &eng, sg_primitive_type primitive, sg_index_type idx_type) {
    _eng = &eng;
    ResourceManager &rm = eng.resource_manager();
    sg_shader shader = rm.get_or_create_shader(*ssao_shader_desc());

    sg_pipeline_desc pip_desc = {0};
    pip_desc.layout.attrs[ATTR_vs_ssao_pos].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.shader = shader;
    pip_desc.primitive_type = primitive;
    pip_desc.index_type = idx_type;
    pip_desc.blend = {.color_attachment_count = 1, .depth_format = SG_PIXELFORMAT_NONE};
    pip_desc.rasterizer = {.cull_mode = SG_CULLMODE_NONE, .face_winding = SG_FACEWINDING_CCW};
    pip_desc.label = "PBR pipeline";
    pip = rm.get_or_create_pipeline(pip_desc);
    // quad vertices
    float quad_vertices[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    bind.vertex_buffers[0] = sg_make_buffer(
        (sg_buffer_desc){.size = sizeof(quad_vertices), .content = quad_vertices, .label = "quad vertices"});

    // placeholder textures
    if (!have_placeholders) {
        create_placeholder_textures();
        have_placeholders = true;
    }

    tex_random = placeholders.random;
    return true;
}

void EffectSSAO::update_bindings() {
    bind.fs_images[SLOT_tex_depth_sampler] = tex_depth;
    bind.fs_images[SLOT_tex_normal_sampler] = tex_normal;
    bind.fs_images[SLOT_tex_random_sampler] = tex_random;
}

void EffectSSAO::apply_uniforms() {
    ssao_params_t ssao_params{
        .u_view = _eng->_camera.inverse_transform(),
        .u_projection = _eng->_camera.projection(),
        .u_inv_projection = math::inverse(_eng->_camera.projection()),
        .radius = radius,
        .noise_scale = math::Vector4f(glengine::window_size().x / 4,
                                      glengine::window_size().y / 4, 0, 0),
        .bias = bias,
    };
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_ssao_params, &ssao_params, sizeof(ssao_params));
}

} // namespace glengine
