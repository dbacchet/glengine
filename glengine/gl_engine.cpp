#include "gl_engine.h"
#include "gl_renderobject.h"

#include "math/vmath.h"
#include "math/math_utils.h"

namespace {

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
}

void scroll_callback(GLFWwindow *window, double xoffs, double yoffs) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    app._camera_manipulator.set_distance(app._camera_manipulator.distance() * (1 + yoffs / 10));
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    math::Vector2i cursor_pos(int(xpos + 0.5), int(ypos + 0.5));
    math::Vector2i cursor_delta(0, 0);
    if (ctx.input_state.previous_cursor_pos != math::Vector2i(-1, -1)) {
        cursor_delta = cursor_pos - ctx.input_state.previous_cursor_pos;
    }
    // rotate view
    if (ctx.input_state.left_button_pressed && ctx.input_state.ctrl_key_pressed == false) {
        app._camera_manipulator.add_azimuth(-0.003f * cursor_delta.x);
        app._camera_manipulator.add_elevation(-0.003f * cursor_delta.y);
    }
    // translate view center
    if (ctx.input_state.middle_button_pressed ||
        (ctx.input_state.left_button_pressed && ctx.input_state.ctrl_key_pressed == true)) {
        float scaling = 0.001f * app._camera_manipulator._distance;
        float dx = scaling * cursor_delta.x;
        float dy = scaling * cursor_delta.y;
        float azimuth = app._camera_manipulator._azimuth;
        math::Vector3f delta(-std::cos(azimuth) * dx - std::sin(azimuth) * dy,
                             -std::sin(azimuth) * dx + std::cos(azimuth) * dy, 0.0f);
        app._camera_manipulator.translate(delta);
    }
    // zoom view
    if (ctx.input_state.right_button_pressed) {
        app._camera_manipulator.set_distance(app._camera_manipulator.distance() * (1 - cursor_delta.y / 100.0f));
    }
    ctx.input_state.previous_cursor_pos = cursor_pos;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
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
} // namespace

namespace glengine {

GLEngine::~GLEngine() {}

bool GLEngine::init(const Config &config) {
    _context = glengine::init_context(1280, 720, "GLEngine sample app");

    _camera.set_perspective(0.1, 100.0, math::utils::deg2rad(45.0f));
    _camera.set_transform(math::create_lookat<float>({-10.0f, -1.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));

    // callbacks for mouse interaction
    glengine::set_callbacks(_context, (void *)this,
                            {
                                scroll_callback,          // scroll_fun_callback
                                mouse_button_callback,    // mousebutton_fun_callback
                                key_callback,             // key_fun_callback
                                cursor_position_callback, // cursorpos_fun_callback
                                nullptr,                  // cursorenterexit_fun_callback
                                nullptr,                  // char_fun_callback
                            });
    return true;
}

bool GLEngine::render() {
    return true;
}

bool GLEngine::terminate() {
    // deallocate all resources
    for (auto it : _meshes) {
        delete it.second;
    }
    _meshes.clear();
    for (auto it : _shaders) {
        delete it.second;
    }
    _shaders.clear();
    glengine::destroy_context(_context);
    return true;
}

Mesh &GLEngine::create_mesh(uint32_t id) {
    Mesh *m = new Mesh();
    _meshes[id] = m;
    return *m;
}

Mesh &GLEngine::get_mesh(uint32_t id) {
    return *_meshes[id];
}

bool GLEngine::has_mesh(uint32_t id) const {
    return _meshes.count(id) > 0;
}

Shader &GLEngine::create_shader(uint32_t id) {
    Shader *s = new Shader();
    _shaders[id] = s;
    return *s;
}

Shader &GLEngine::get_shader(uint32_t id) {
    return *_shaders[id];
}

bool GLEngine::has_shader(uint32_t id) const {
    return _shaders.count(id) > 0;
}

} // namespace glengine
