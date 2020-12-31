#include "gl_engine.h"
#include "gl_logger.h"

#include "math/vmath.h"
#include "math/math_utils.h"

#include "sokol_gfx.h"
#include "sokol_time.h"
#include "sokol_gfx_imgui.h"
#include "imgui/imgui.h"
#include "sokol_imgui.h"
#include "imgui/imgui_impl_glfw.h"

#include "shaders/generated/multipass-basic.glsl.h"

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
        sg_pass_action pass_action = {0}; // only the pass action since the target is teh default framebuffer
        sg_pipeline pip;
        sg_bindings bind;
    } fsq;
    sg_imgui_t sg_imgui;
};

} // namespace glengine

namespace {

// called initially and when window size changes
void create_offscreen_pass(glengine::State &state, int width, int height) {
    // destroy previous resource (can be called for invalid id)
    sg_destroy_pass(state.offscreen.pass.pass_id);
    sg_destroy_image(state.offscreen.pass.pass_desc.color_attachments[0].image);
    sg_destroy_image(state.offscreen.pass.pass_desc.depth_stencil_attachment.image);
    // create offscreen rendertarget images and pass
    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? 4 : 1;
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
                                    .pixel_format = SG_PIXELFORMAT_DEPTH,
                                    .sample_count = offscreen_sample_count,
                                    .min_filter = SG_FILTER_LINEAR,
                                    .mag_filter = SG_FILTER_LINEAR,
                                    .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                    .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                    .label = "depth image"};
    state.offscreen.pass.pass_desc = (sg_pass_desc){.color_attachments =
                                                        {
                                                            [0].image = sg_make_image(&color_img_desc),
                                                        },
                                                    .depth_stencil_attachment.image = sg_make_image(&depth_img_desc),
                                                    .label = "offscreen pass"};
    state.offscreen.pass.pass_id = sg_make_pass(&state.offscreen.pass.pass_desc);
    // pass action for offscreen pass
    state.offscreen.pass.pass_action =
        (sg_pass_action){.colors = {
                             [0] = {.action = SG_ACTION_CLEAR, .val = {0.1f, 0.1f, 0.1f, 1.0f}},
                         }};
    // also need to update the fullscreen-quad texture bindings
    state.fsq.bind.fs_images[0] = state.offscreen.pass.pass_desc.color_attachments[0].image;
}

// create the final fullscreen quad rendering pass
void create_fsq_pass(glengine::State &state, int width, int height) {
    state.fsq.pass_action = {.colors[0] = {.action = SG_ACTION_CLEAR, .val = {0.1f, 0.1f, 0.1f, 1.0f}}};
    // fulscreen quad rendering
    float quad_vertices[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    sg_buffer quad_vbuf = sg_make_buffer(
        (sg_buffer_desc){.size = sizeof(quad_vertices), .content = quad_vertices, .label = "quad vertices"});
    // the pipeline object to render the fullscreen quad
    state.fsq.pip =
        sg_make_pipeline((sg_pipeline_desc){.layout = {.attrs[ATTR_vs_fsq_pos].format = SG_VERTEXFORMAT_FLOAT2},
                                            .shader = sg_make_shader(fsq_shader_desc()),
                                            .primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP,
                                            .label = "fullscreen quad pipeline"});
    // resource bindings to render a fullscreen quad
    state.fsq.bind = (sg_bindings){.vertex_buffers[0] = quad_vbuf,
                                   .fs_images = {
                                       [SLOT_tex0] = state.offscreen.pass.pass_desc.color_attachments[0].image,
                                   }};
}

int save_screenshot(const char *filename) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    int x = viewport[0];
    int y = viewport[1];
    int width = viewport[2];
    int height = viewport[3];

    char *data = (char *)malloc((size_t)(width * height * 3)); // 3 components (R, G, B)

    if (!data)
        return 0;

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_flip_vertically_on_write(true);
    int saved = stbi_write_png(filename, width, height, 3, data, 0);

    free(data);

    return saved;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
        ctx.input_state.ctrl_key_pressed = action == GLFW_PRESS;
    }
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
        ctx.input_state.shift_key_pressed = action == GLFW_PRESS;
    }
    if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT) {
        ctx.input_state.alt_key_pressed = action == GLFW_PRESS;
    }
    // statistics
    if (key == GLFW_KEY_F5 && action == GLFW_RELEASE) {
        app._config.show_imgui_statistics = !app._config.show_imgui_statistics;
    }
    // buffers and tuning
    if (key == GLFW_KEY_F6 && action == GLFW_RELEASE) {
        app._config.show_framebuffer_texture = !app._config.show_framebuffer_texture;
    }
}

void scroll_callback(GLFWwindow *window, double xoffs, double yoffs) {
    if (ImGui::GetIO().WantCaptureMouse) {
        // do nothing if the mouse is on top a UI element
        return;
    }
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    app._camera_manipulator.set_distance(app._camera_manipulator.distance() * (1 - yoffs / 10));
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    auto &cm = app._camera_manipulator;
    math::Vector2i cursor_pos(int(xpos + 0.5), int(ypos + 0.5));
    math::Vector2i cursor_delta(0, 0);
    if (ctx.input_state.previous_cursor_pos != math::Vector2i(-1, -1)) {
        cursor_delta = cursor_pos - ctx.input_state.previous_cursor_pos;
    }
    // rotate view
    if (ctx.input_state.left_button_pressed && ctx.input_state.ctrl_key_pressed == false) {
        cm.add_azimuth(-0.003f * cursor_delta.x);
        // cm.add_elevation(-0.003f * cursor_delta.y);
        cm.set_elevation(math::utils::clamp<float>(cm.elevation() - 0.003f * cursor_delta.y, 0, M_PI));
    }
    // translate view center
    if (ctx.input_state.middle_button_pressed ||
        (ctx.input_state.left_button_pressed && ctx.input_state.ctrl_key_pressed == true)) {
        float scaling = 0.001f * cm.distance();
        float dx = scaling * cursor_delta.x;
        float dy = scaling * cursor_delta.y;
        float azimuth = cm.azimuth();
        math::Vector3f delta(-std::cos(azimuth) * dx - std::sin(azimuth) * dy,
                             -std::sin(azimuth) * dx + std::cos(azimuth) * dy, 0.0f);
        cm.translate(delta);
    }
    // zoom view
    if (ctx.input_state.right_button_pressed) {
        cm.set_distance(cm.distance() * (1 - cursor_delta.y / 100.0f));
    }
    ctx.input_state.previous_cursor_pos = cursor_pos;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse) {
        // do nothing if the mouse is on top a UI element
        return;
    }
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx.input_state.left_button_pressed = action == GLFW_PRESS;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        ctx.input_state.middle_button_pressed = action == GLFW_PRESS;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        ctx.input_state.right_button_pressed = action == GLFW_PRESS;
    }
}

void window_size_callback(GLFWwindow *window, int width, int height) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    ctx.window_state.window_size = {width, height};
    printf("win size: %d %d\n", width, height);
}

void framebuffer_size_callback(GLFWwindow *window, int fb_width, int fb_height) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    ctx.window_state.framebuffer_size = {fb_width, fb_height};
    printf("fb size: %d %d\n", fb_width, fb_height);
}

} // namespace

namespace glengine {

GLEngine::~GLEngine() {}

bool GLEngine::init(const Config &config) {

    MicroProfileOnThreadCreate("Main");
    // turn on profiling
    MicroProfileSetEnableAllGroups(true);
    MicroProfileSetForceMetaCounters(true);

    // gfx context
    _config = config;
    _context = glengine::init_context(config, "GLEngine sample app", (void *)this,
                                      {
                                          scroll_callback,          // scroll_fun_callback
                                          mouse_button_callback,    // mousebutton_fun_callback
                                          key_callback,             // key_fun_callback
                                          cursor_position_callback, // cursorpos_fun_callback
                                          nullptr,                  // cursorenterexit_fun_callback
                                          nullptr,                  // char_fun_callback
                                          window_size_callback,
                                          framebuffer_size_callback //
                                      });
    // resource manager
    _resource_manager.init();
    // state needed by the sokol renderer
    _state = new State;

    // init sokol-gfx
    sg_setup((sg_desc){0});
    stm_setup();
    // use sokol-imgui with all default-options
    simgui_desc_t simgui_desc = {};
    simgui_desc.dpi_scale = _context.window_state.framebuffer_size.x / _context.window_state.window_size.x;
    simgui_setup(&simgui_desc);

    sg_imgui_init(&_state->sg_imgui);

    // ////// //
    // passes //
    // ////// //
    // create offscreen pass
    create_offscreen_pass(*_state, _context.window_state.window_size.x, _context.window_state.window_size.y);
    // final pass
    create_fsq_pass(*_state, _context.window_state.window_size.x, _context.window_state.window_size.y);

    // create root of the scene
    _root = new Object();
    // sensible defaults for the view
    _camera.set_perspective(0.1, 1000.0, math::utils::deg2rad(45.0f));
    _camera.set_transform(math::create_lookat<float>({-10.0f, -1.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));

    return true;
}

bool GLEngine::render() {
    MICROPROFILE_SCOPEI("glengine", "render", MP_AUTO);
    const auto &winsize = _context.window_state.window_size;
    const auto &fbsize = _context.window_state.framebuffer_size;
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

    // ////////// //
    // final pass //
    // ////////// //
    MICROPROFILE_ENTERI("glengine", "final pass", MP_AUTO);
    // render to final framebuffer (using sokol low-level calls directly)
    sg_begin_default_pass(&_state->fsq.pass_action, fbsize.x, fbsize.y);
    sg_apply_pipeline(_state->fsq.pip);
    sg_apply_bindings(&_state->fsq.bind);
    sg_draw(0, 4, 1);

    // Start the Dear ImGui frame
    MICROPROFILE_ENTERI("glengine", "imgui", MP_AUTO);
    ImGui_ImplGlfw_NewFrame();
    simgui_new_frame(fbsize.x, fbsize.y, delta_time);
    // statistics and debug  
    if (_config.show_imgui_statistics) {
        ImGui::ShowMetricsWindow();
    }
    if (_config.show_framebuffer_texture) {
        int img_width = 200;
        int img_height = img_width * (float)fbsize.y / fbsize.x;
        ImGui::Begin("fb images");
        ImGui::Text("offscreen color attach 0");
        ImGui::Image(
            (void *)(uintptr_t)_state->offscreen.pass.pass_desc.color_attachments[0].image.id,
            ImVec2(img_width, img_height), ImVec2(0, 1),
            ImVec2(1, 0)); // (ImVec2){64,64}, (ImVec2){0,0}, (ImVec2){1,1}, (ImVec4){1,1,1,1}, (ImVec4){1,1,1,1});
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
    sg_commit();

    glfwSwapBuffers(_context.window);
    glfwPollEvents();

    MicroProfileFlip(0);

    return !glfwWindowShouldClose(_context.window);
}

bool GLEngine::terminate() {
    log_info("Shutting down engine");
    MicroProfileShutdown();
    // destroying objects
    delete _root;
    // deallocate all resources
    _resource_manager.terminate();
    // destroy the renderer's state
    delete _state;
    // destroy the gfx context
    glengine::destroy_context(_context);
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

void GLEngine::add_ui_function(std::function<void(void)> fun) {
    _ui_functions.push_back(fun);
}

} // namespace glengine
