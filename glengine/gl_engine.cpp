#include "gl_engine.h"
#include "gl_object.h"
#include "gl_logger.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "math/vmath.h"
#include "math/math_utils.h"

#include "stb/stb_image_write.h"
#include "microprofile/microprofile.h"

#include <random>


namespace {

int saveScreenshot(const char *filename)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    int x = viewport[0];
    int y = viewport[1];
    int width = viewport[2];
    int height = viewport[3];

    char *data = (char*) malloc((size_t) (width * height * 3)); // 3 components (R, G, B)

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
    // printf("win size: %d %d\n", width, height);
}

void framebuffer_size_callback(GLFWwindow *window, int fb_width, int fb_height) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    ctx.window_state.framebuffer_size = {fb_width, fb_height};
    // printf("fb size: %d %d\n", fb_width, fb_height);
    app.resize_buffers();
}

} // namespace

namespace glengine {

GLEngine::~GLEngine() {}

bool GLEngine::init(const Config &config) {

	MicroProfileOnThreadCreate("Main");
	//turn on profiling
	MicroProfileSetEnableAllGroups(true);
	MicroProfileSetForceMetaCounters(true);

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

    _camera.set_perspective(0.1, 1000.0, math::utils::deg2rad(45.0f));
    _camera.set_transform(math::create_lookat<float>({-10.0f, -1.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));

    // initialize the resource manager (creates default resources)
    _resource_manager.init();
    // create root of the scene
    _root = new Object();

    const float srgb_gamma = 2.2;
    glClearColor(pow(0.1f,srgb_gamma), pow(0.1f,srgb_gamma), pow(0.1f,srgb_gamma), 1.0f);
    glEnable(GL_DEPTH_TEST);

    return true;
}

bool GLEngine::render() {
    MICROPROFILE_SCOPEI("glengine","render",MP_AUTO);
    const auto &fbsize = _context.window_state.framebuffer_size;

    _camera_manipulator.update(_camera);
    _camera.update(fbsize.x, fbsize.y);

    const float srgb_gamma = 2.2;
    const math::Vector4f clear_color = {std::pow(0.1f,srgb_gamma), std::pow(0.1f,srgb_gamma), std::pow(0.1f,srgb_gamma), 1.0f};

    glViewport(0, 0, fbsize.x, fbsize.y);
    glEnable(GL_FRAMEBUFFER_SRGB); 
    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    MICROPROFILE_ENTERI("glengine","draw_commands",MP_AUTO);
    // create the draw commands for the scene
    if (_root) {
        _root->draw(_renderer, _camera);
    }
    MICROPROFILE_LEAVE();

    // render to g-buffer (apply the draw commands)
    MICROPROFILE_ENTERI("glengine","render",MP_AUTO);
    log_debug("render items %lu",_renderer.render_items.size());
    _renderer.render();
    _renderer.render_items.clear();
    MICROPROFILE_LEAVE();

    // Start the Dear ImGui frame
    glDisable(GL_FRAMEBUFFER_SRGB);
    MICROPROFILE_ENTERI("glengine","imgui",MP_AUTO);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (_config.show_imgui_statistics) {
        ImGui::ShowMetricsWindow();
    }
    for (auto& fun: _ui_functions) {
        fun();
    }
    // render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    MICROPROFILE_LEAVE();

    glfwSwapBuffers(_context.window);
    glfwPollEvents();
	
    MicroProfileFlip(0);

    return !glfwWindowShouldClose(_context.window);
}

bool GLEngine::terminate() {
	MicroProfileShutdown();
    // deallocate all resources
    delete _root;
    _root = nullptr;
    glengine::destroy_context(_context);
    return true;
}

Object *GLEngine::create_renderobject(Object *parent, ID id) {
    Object *ro = new Object(parent ? parent : _root, id);
    return ro;
}

Object *GLEngine::create_renderobject(const Renderable &renderable, Object *parent, ID id) {
    Object *ro = create_renderobject(parent, id);
    ro->add_renderable(&renderable, 1);
    return ro;
}

Object *GLEngine::create_renderobject(const std::vector<Renderable> &renderables, Object *parent, ID id) {
    Object *ro = create_renderobject(parent, id);
    ro->init(renderables);
    return ro;
}

void GLEngine::add_ui_function(std::function<void(void)> fun) {
    _ui_functions.push_back(fun);
}

math::Vector2i GLEngine::cursor_pos() const {
    return _context.input_state.previous_cursor_pos;
}

ID GLEngine::object_at_screen_coord(const math::Vector2i &cursor_pos) const {
    // cursor is in window coord, so we need to convert to framebuffer
    float scale_x = float(_context.window_state.framebuffer_size.x)/_context.window_state.window_size.x;
    float scale_y = float(_context.window_state.framebuffer_size.y)/_context.window_state.window_size.y;
    int32_t px = int32_t(scale_x * cursor_pos.x + 0.5f);
    int32_t py = int32_t(scale_y * (_context.window_state.window_size.y -1 - cursor_pos.y) + 0.5f);
    int32_t idx = py*_context.window_state.framebuffer_size.x+px;
    if (idx<0 || idx>=(int32_t)_id_buffer.size()) {
        return NULL_ID;
    }
    return _id_buffer[idx];
}

void GLEngine::save_screenshot(const char *filename) {
    saveScreenshot(filename);
}

void GLEngine::resize_buffers() {
    // const auto &win_size = _context.window_state.window_size;
    const auto &fb_size = _context.window_state.framebuffer_size;
    _id_buffer.resize(fb_size.x*fb_size.y, NULL_ID);
}

} // namespace glengine
