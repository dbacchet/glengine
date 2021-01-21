#include "gl_engine.h"
#include "gl_context.h"
#include "gl_logger.h"

#include "math/vmath.h"
#include "math/math_utils.h"

#include "sokol_gfx.h"
#include "sokol_time.h"
#include "sokol_gfx_imgui.h"
#include "imgui/imgui.h"
#include "sokol_imgui.h"

#include "gl_material.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"
#include "shaders/generated/multipass-basic.glsl.h"
#include "gl_effect_ssao.h"
#include "gl_effect_blur.h"

#include "stb/stb_image_write.h"
#include "microprofile/microprofile.h"

#include <random>

namespace glengine {

struct Pass {
    sg_pass_action pass_action = {0};
    sg_pass_desc pass_desc = {0};
    sg_pass pass_id = {0};
};

struct State {
    struct {
        Pass pass;
    } offscreen;
    struct {
        Pass pass;
        EffectSSAO effect;
        Pass blur_pass;
        EffectBlur effect_blur;
        bool enabled = true;
    } ssao;
    struct {
        sg_pass_action pass_action = {0}; // only the pass action since the target is the default framebuffer
        sg_pipeline pip = {0};
        sg_bindings bind = {0};
        bool debug = false;
    } fsq;
    sg_imgui_t sg_imgui;
    sg_image default_textures[glengine::ResourceManager::DefaultImageNum] = {0};
};


GLEngine::~GLEngine() {}

bool GLEngine::init(const Config &config) {

    MicroProfileOnThreadCreate("Main");
    // turn on profiling
    MicroProfileSetEnableAllGroups(true);
    MicroProfileSetForceMetaCounters(true);

    // gfx context
    _config = config;
    if (!glengine::init_context(config, "GLEngine sample app", (void *)this)) {
        log_error("Error creating the rendering context");
        return false;
    }
    // state needed by the sokol renderer
    _state = new State;

    // init sokol-gfx
    sg_setup((sg_desc){0});
    stm_setup();
    // use sokol-imgui with all default-options
    simgui_desc_t simgui_desc = {.ini_filename = "imgui.ini"};
    simgui_desc.dpi_scale = framebuffer_size().x / window_size().x;
    simgui_setup(&simgui_desc);

    sg_imgui_init(&_state->sg_imgui);

    // resource manager
    _resource_manager.init();
    // add standard resources
    for (int i = 0; i < ResourceManager::DefaultImageNum; i++) {
        _state->default_textures[i] = _resource_manager.default_image((ResourceManager::DefaultImage)i);
    }

    // ////// //
    // passes //
    // ////// //
    // create offscreen pass
    create_offscreen_pass();
    // ssao pass
    create_ssao_pass();
    // final pass
    create_fsq_pass();

    // create root of the scene
    _root = new Object();
    // sensible defaults for the view
    _camera.set_perspective(1.0f, 100.0f, math::utils::deg2rad(45.0f));
    _camera.set_transform(math::create_lookat<float>({-10.0f, -1.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));

    return true;
}

bool GLEngine::render() {
    glengine::begin_frame();
    MICROPROFILE_SCOPEI("glengine", "render", MP_AUTO);
    const auto &winsize = window_size();
    const auto &fbsize = framebuffer_size();
    const double delta_time = stm_sec(stm_laptime(&_curr_time));

    _camera_manipulator.update(_camera);
    _camera.update(fbsize.x, fbsize.y);

    // /////////////////// //
    // main offscreen pass //
    // /////////////////// //
    MICROPROFILE_ENTERI("glengine", "offscreen pass", MP_AUTO);
    sg_begin_pass(_state->offscreen.pass.pass_id, &_state->offscreen.pass.pass_action);

    /// \todo this is inefficient because there is no pipeline state caching - replace with a proper renderer that
    /// implements draw call sorting and optimization
    _root->draw(_camera, math::matrix4_identity<float>());

    sg_end_pass();
    MICROPROFILE_LEAVE();

    // //// //
    // ssao //
    // //// //
    if (_config.use_mrt) {
        MICROPROFILE_ENTERI("glengine", "ssao pass", MP_AUTO);
        sg_begin_pass(_state->ssao.pass.pass_id, &_state->ssao.pass.pass_action);
        if (_state->ssao.enabled) {

            sg_apply_pipeline(_state->ssao.effect.pip);
            sg_apply_bindings(_state->ssao.effect.bind);
            _state->ssao.effect.apply_uniforms();
            sg_draw(0, 4, 1);
        }
        sg_end_pass();
        MICROPROFILE_LEAVE();
        MICROPROFILE_ENTERI("glengine", "ssao blur pass", MP_AUTO);
        sg_begin_pass(_state->ssao.blur_pass.pass_id, &_state->ssao.blur_pass.pass_action);
        if (_state->ssao.enabled) {

            sg_apply_pipeline(_state->ssao.effect_blur.pip);
            sg_apply_bindings(_state->ssao.effect_blur.bind);
            _state->ssao.effect_blur.apply_uniforms();
            sg_draw(0, 4, 1);
        }
        sg_end_pass();
        MICROPROFILE_LEAVE();
    }

    // ////////// //
    // final pass //
    // ////////// //
    MICROPROFILE_ENTERI("glengine", "final pass", MP_AUTO);
    // render to final framebuffer (using sokol low-level calls directly)
    sg_begin_default_pass(&_state->fsq.pass_action, fbsize.x, fbsize.y);
    sg_apply_pipeline(_state->fsq.pip);
    sg_apply_bindings(&_state->fsq.bind);
    fsq_params_t fsq_params = {_state->fsq.debug ? 1.0f : 0.0f, _camera.near_plane(), _camera.far_plane()};
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fsq_params, &fsq_params, sizeof(fsq_params));
    sg_draw(0, 4, 1);

    // Start the Dear ImGui frame
    MICROPROFILE_ENTERI("glengine", "imgui", MP_AUTO);
    simgui_new_frame(fbsize.x, fbsize.y, delta_time);
    // statistics and debug
    if (_config.show_imgui_statistics) {
        ImGui::ShowMetricsWindow();
    }
    if (_config.show_framebuffer_texture) {
        int img_width = 200;
        int img_height = img_width * (float)fbsize.y / fbsize.x;
        ImGui::Begin("fb images");
        ImGui::Checkbox("Debug visualization", &_state->fsq.debug);
        if (_state->offscreen.pass.pass_desc.color_attachments[0].image.id) {
            ImGui::Text("offscreen attach 0 (color)");
            ImGui::Image((void *)(uintptr_t)_state->offscreen.pass.pass_desc.color_attachments[0].image.id,
                         ImVec2(img_width, img_height), ImVec2(0, 1), ImVec2(1, 0));
        }
        if (_state->offscreen.pass.pass_desc.color_attachments[1].image.id) {
            ImGui::Text("offscreen attach 1 (normal)");
            ImGui::Image((void *)(uintptr_t)_state->offscreen.pass.pass_desc.color_attachments[1].image.id,
                         ImVec2(img_width, img_height), ImVec2(0, 1), ImVec2(1, 0));
        }
        if (_state->offscreen.pass.pass_desc.color_attachments[2].image.id) {
            ImGui::Text("offscreen attach 2 (depth)");
            ImGui::Image((void *)(uintptr_t)_state->offscreen.pass.pass_desc.color_attachments[2].image.id,
                         ImVec2(img_width, img_height), ImVec2(0, 1), ImVec2(1, 0));
        }
        if (_config.use_mrt) {
            ImGui::Text("SSAO");
            ImGui::Checkbox("ssao enabled", &_state->ssao.enabled);
            if (_state->ssao.enabled && _state->ssao.pass.pass_desc.color_attachments[0].image.id) {
                ImGui::Image((void *)(uintptr_t)_state->ssao.pass.pass_desc.color_attachments[0].image.id,
                             ImVec2(img_width, img_height), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::DragFloat("ssao bias", &_state->ssao.effect.bias, 0.001, -0.1, 0.1);
                ImGui::DragFloat("ssao radius", &_state->ssao.effect.radius, 0.001, -1, 1);
            }
        }
        ImGui::End();
    }
    // user ui functions
    for (auto &fun : _ui_functions) {
        fun();
    }
    // render ImGui
    sg_imgui_draw(&_state->sg_imgui);
    simgui_render();
    MICROPROFILE_LEAVE();
    sg_end_pass();
    MICROPROFILE_LEAVE();
    // finalize frame
    MICROPROFILE_ENTERI("glengine", "commit", MP_AUTO);
    sg_commit();
    MICROPROFILE_LEAVE();

    MICROPROFILE_ENTERI("glengine", "swapbuffers", MP_AUTO);
    glengine::end_frame();
    MICROPROFILE_LEAVE();

    MicroProfileFlip(0);

    return !glengine::window_should_close();
}

bool GLEngine::terminate() {
    log_info("Shutting down engine");
    MicroProfileShutdown();
    // destroying objects
    log_info("Glengine: delete objects");
    delete _root;
    // deallocate all resources
    log_info("Glengine: shut down resource manager");
    _resource_manager.terminate();
    // destroy the renderer's state
    log_info("Glengine: delete internal state");
    delete _state;
    // destroy the gfx context
    log_info("Glengine: destroy gfx context");
    glengine::destroy_context();
    return true;
}

Object *GLEngine::create_object(Object *parent, ID id) {
    Object *ro = new Object(parent ? parent : _root, id);
    return ro;
}

Object *GLEngine::create_object(const Renderable &renderable, Object *parent, ID id) {
    Object *ro = create_object(parent, id);
    ro->add_renderable(&renderable, 1);
    return ro;
}

Object *GLEngine::create_object(const std::vector<Renderable> &renderables, Object *parent, ID id) {
    Object *ro = create_object(parent, id);
    ro->init(renderables);
    return ro;
}

Mesh *GLEngine::create_mesh() {
    Mesh *mesh = new Mesh();
    _resource_manager.register_mesh(mesh);
    return mesh;
}

Mesh *GLEngine::create_mesh(const std::vector<Vertex> &vertices_, const std::vector<uint32_t> &indices_,
                            sg_usage usage) {
    Mesh *mesh = create_mesh();
    mesh->init(vertices_, indices_, usage);
    return mesh;
}

Mesh *GLEngine::create_axis_mesh() {
    Mesh *m = create_mesh();
    MeshData md = create_axis_data();
    m->init(md.vertices, md.indices);
    return m;
}

Mesh *GLEngine::create_quad_mesh() {
    Mesh *m = create_mesh();
    MeshData md = create_quad_data();
    m->init(md.vertices, md.indices);
    return m;
}

Mesh *GLEngine::create_box_mesh(const math::Vector3f &size) {
    Mesh *m = create_mesh();
    MeshData md = create_box_data(size);
    m->init(md.vertices, md.indices);
    return m;
}

Mesh *GLEngine::create_sphere_mesh(float radius, uint32_t subdiv) {
    Mesh *m = create_mesh();
    MeshData md = create_sphere_data(radius, subdiv);
    m->init(md.vertices, md.indices);
    return m;
}

Mesh *GLEngine::create_grid_mesh(float len, float step) {
    Mesh *m = create_mesh();
    MeshData md = create_grid_data(len, step);
    m->init(md.vertices, md.indices);
    return m;
}

void GLEngine::add_ui_function(std::function<void(void)> fun) {
    _ui_functions.push_back(fun);
}

// called initially and when window size changes
void GLEngine::create_offscreen_pass() {
    glengine::State &state = *_state;
    const int width = window_size().x;
    const int height = window_size().y;
    const int msaa_samples = _config.msaa_samples;
    // destroy previous resource (can be called for invalid id)
    sg_destroy_pass(state.offscreen.pass.pass_id);
    sg_destroy_image(state.offscreen.pass.pass_desc.color_attachments[0].image);
    sg_destroy_image(state.offscreen.pass.pass_desc.color_attachments[1].image);
    sg_destroy_image(state.offscreen.pass.pass_desc.color_attachments[2].image);
    sg_destroy_image(state.offscreen.pass.pass_desc.depth_stencil_attachment.image);
    // create offscreen rendertarget images and pass.
    // This pass can use multiple rendertargets:
    // - regular color
    // - normals in view space (3 coords encoded in rgb using normal.x*0.5+0.5 like normal maps)
    // - depth in view space (float encoded in rgba)
    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? msaa_samples : 1;
    sg_image_desc color_img_desc = {.render_target = true,
                                    .width = width,
                                    .height = height,
                                    .sample_count = offscreen_sample_count,
                                    .min_filter = SG_FILTER_LINEAR,
                                    .mag_filter = SG_FILTER_LINEAR,
                                    .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                    .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                    .label = "color image"};
    sg_image_desc depth_img_desc = {.render_target = true,
                                    .width = width,
                                    .height = height,
                                    .pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL,
                                    .sample_count = offscreen_sample_count,
                                    .min_filter = SG_FILTER_LINEAR,
                                    .mag_filter = SG_FILTER_LINEAR,
                                    .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                    .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                    .label = "depth image"};
    state.offscreen.pass.pass_desc = {0}; // can't use struct initializer because arrays are not supported in c++
    state.offscreen.pass.pass_desc.color_attachments[0].image = sg_make_image(&color_img_desc); // color info
    state.offscreen.pass.pass_desc.depth_stencil_attachment.image = sg_make_image(&depth_img_desc);
    state.offscreen.pass.pass_desc.label = "offscreen pass";
    // enable MRT
    if (_config.use_mrt) {
        sg_image_desc normal_rgba_img_desc = color_img_desc;
        normal_rgba_img_desc.label = "normal rgba image";
        sg_image_desc depth_rgba_img_desc = color_img_desc;
        depth_rgba_img_desc.label = "depth rgba image";
        state.offscreen.pass.pass_desc.color_attachments[1].image = sg_make_image(&normal_rgba_img_desc);
        state.offscreen.pass.pass_desc.color_attachments[2].image = sg_make_image(&depth_rgba_img_desc);
    }
    state.offscreen.pass.pass_id = sg_make_pass(&state.offscreen.pass.pass_desc);
    // pass action and update fullscreen quad bindings
    state.offscreen.pass.pass_action = {};
    state.offscreen.pass.pass_action.colors[0] = {.action = SG_ACTION_CLEAR, .val = {0.1f, 0.1f, 0.1f, 1.0f}};
    state.fsq.bind.fs_images[0] = state.offscreen.pass.pass_desc.color_attachments[0].image;
    if (_config.use_mrt) {
        state.fsq.bind.fs_images[1] = state.offscreen.pass.pass_desc.color_attachments[1].image;
        state.fsq.bind.fs_images[2] = state.offscreen.pass.pass_desc.color_attachments[2].image;
        state.offscreen.pass.pass_action.colors[1] = {.action = SG_ACTION_CLEAR,
                                                      .val = {0.5f, 0.5f, 0.5f, 1.0f}}; // null normal
        state.offscreen.pass.pass_action.colors[2] = {.action = SG_ACTION_CLEAR,
                                                      .val = {1.0f, 1.0f, 1.0f, 1.0f}}; // far plane
    }
}

// create ssao pass
void GLEngine::create_ssao_pass() {
    // ssao
    glengine::State &state = *_state;
    const int width = window_size().x;
    const int height = window_size().y;
    // destroy previous resource (can be called for invalid id)
    sg_destroy_pass(state.ssao.pass.pass_id);
    sg_destroy_image(state.ssao.pass.pass_desc.color_attachments[0].image);
    sg_image_desc color_img_desc = {.render_target = true,
                                    .width = width,
                                    .height = height,
                                    .sample_count = 1,
                                    .min_filter = SG_FILTER_LINEAR,
                                    .mag_filter = SG_FILTER_LINEAR,
                                    .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                    .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                    .label = "color image"};
    state.ssao.pass.pass_desc = {0}; // can't use struct initializer because arrays are not supported in c++
    state.ssao.pass.pass_desc.color_attachments[0].image = sg_make_image(&color_img_desc); // color info
    state.ssao.pass.pass_desc.label = "ssao pass";
    state.ssao.pass.pass_id = sg_make_pass(&state.ssao.pass.pass_desc);
    // pass action for ssao pass
    state.ssao.pass.pass_action = {};
    state.ssao.pass.pass_action.colors[0] = {.action = SG_ACTION_CLEAR, .val = {1.0f, 1.0f, 1.0f, 1.0f}};
    // initialize the effect
    state.ssao.effect.init(*this, SG_PRIMITIVETYPE_TRIANGLE_STRIP);
    state.ssao.effect.tex_normal = state.offscreen.pass.pass_desc.color_attachments[1].image;
    state.ssao.effect.tex_depth = state.offscreen.pass.pass_desc.color_attachments[2].image;
    state.ssao.effect.update_bindings();
    // blur
    // destroy previous resource (can be called for invalid id)
    sg_destroy_pass(state.ssao.blur_pass.pass_id);
    sg_destroy_image(state.ssao.blur_pass.pass_desc.color_attachments[0].image);
    color_img_desc.label = "blur color image";
    state.ssao.blur_pass.pass_desc = {0};
    state.ssao.blur_pass.pass_desc.color_attachments[0].image = sg_make_image(&color_img_desc); // final buffer
    state.ssao.blur_pass.pass_desc.label = "ssao blur pass";
    state.ssao.blur_pass.pass_id = sg_make_pass(&state.ssao.blur_pass.pass_desc);
    // blur_pass action for ssao blur_pass
    state.ssao.blur_pass.pass_action = {};
    state.ssao.blur_pass.pass_action.colors[0] = {.action = SG_ACTION_CLEAR, .val = {1.0f, 1.0f, 1.0f, 1.0f}};
    // initialize the effect
    state.ssao.effect_blur.init(*this, SG_PRIMITIVETYPE_TRIANGLE_STRIP);
    state.ssao.effect_blur.tex = state.ssao.pass.pass_desc.color_attachments[0].image;
    state.ssao.effect_blur.update_bindings();
    // also need to update the fullscreen-quad texture bindings
    state.fsq.bind.fs_images[3] = state.ssao.blur_pass.pass_desc.color_attachments[0].image;
}

// create the final fullscreen quad rendering pass
void GLEngine::create_fsq_pass() {
    glengine::State &state = *_state;
    const int width = window_size().x;
    const int height = window_size().y;
    state.fsq.pass_action = {0};
    state.fsq.pass_action.colors[0] = {.action = SG_ACTION_CLEAR, .val = {0.1f, 0.1f, 0.1f, 1.0f}};
    // fulscreen quad rendering
    float quad_vertices[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    sg_buffer quad_vbuf = sg_make_buffer(
        (sg_buffer_desc){.size = sizeof(quad_vertices), .content = quad_vertices, .label = "quad vertices"});
    // the pipeline object to render the fullscreen quad
    sg_pipeline_desc fsq_pip_desc = {0};
    fsq_pip_desc.layout.attrs[ATTR_vs_fsq_pos].format = SG_VERTEXFORMAT_FLOAT2;
    fsq_pip_desc.shader = sg_make_shader(fsq_shader_desc());
    fsq_pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    fsq_pip_desc.label = "fullscreen quad pipeline";
    state.fsq.pip = sg_make_pipeline(fsq_pip_desc);
    // resource bindings to render a fullscreen quad
    state.fsq.bind = {0};
    state.fsq.bind.vertex_buffers[0] = quad_vbuf;
    state.fsq.bind.fs_images[SLOT_tex0] = state.offscreen.pass.pass_desc.color_attachments[0].image;
    if (_config.use_mrt) {
        state.fsq.bind.fs_images[SLOT_tex_normal] = state.offscreen.pass.pass_desc.color_attachments[1].image;
        state.fsq.bind.fs_images[SLOT_tex_depth] = state.offscreen.pass.pass_desc.color_attachments[2].image;
        state.fsq.bind.fs_images[SLOT_tex_ssao] = state.ssao.blur_pass.pass_desc.color_attachments[0].image.id
                                                      ? state.ssao.blur_pass.pass_desc.color_attachments[0].image
                                                      : state.default_textures[glengine::ResourceManager::White];
    } else {
        state.fsq.bind.fs_images[SLOT_tex_normal] = state.default_textures[glengine::ResourceManager::Normal];
        state.fsq.bind.fs_images[SLOT_tex_depth] = state.default_textures[glengine::ResourceManager::White];
        state.fsq.bind.fs_images[SLOT_tex_ssao] = state.default_textures[glengine::ResourceManager::White];
    }
}

} // namespace glengine
