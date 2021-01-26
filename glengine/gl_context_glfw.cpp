#include "gl_context_glfw.h"

#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "math/vmath.h"
#include "math/math_utils.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"

#include "stb/stb_image_write.h"
#include "gl_engine.h"

#include <cstdlib>
#include <cstdio>

namespace glengine {

// input states that are needed across different callbacks
struct InputState {
    bool left_button_pressed = false;
    bool middle_button_pressed = false;
    bool right_button_pressed = false;
    bool ctrl_key_pressed = false;
    bool shift_key_pressed = false;
    bool alt_key_pressed = false;
    math::Vector2i previous_cursor_pos{-1,-1};
};

// information about the current window
struct WindowState {
    math::Vector2i window_size{-1,-1};
    math::Vector2i framebuffer_size{-1,-1};
};

struct GLFWContext {
    GLFWwindow *window = nullptr;
    InputState input_state;
    WindowState window_state;
};

} //

namespace {

// global application context
glengine::GLFWContext g_context;

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
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
        g_context.input_state.ctrl_key_pressed = action == GLFW_PRESS;
    }
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
        g_context.input_state.shift_key_pressed = action == GLFW_PRESS;
    }
    if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT) {
        g_context.input_state.alt_key_pressed = action == GLFW_PRESS;
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
    auto &cm = app._camera_manipulator;
    math::Vector2i cursor_pos(int(xpos + 0.5), int(ypos + 0.5));
    math::Vector2i cursor_delta(0, 0);
    if (g_context.input_state.previous_cursor_pos != math::Vector2i(-1, -1)) {
        cursor_delta = cursor_pos - g_context.input_state.previous_cursor_pos;
    }
    // rotate view
    if (g_context.input_state.left_button_pressed && g_context.input_state.ctrl_key_pressed == false && g_context.input_state.shift_key_pressed == false) {
        cm.add_azimuth(-0.003f * cursor_delta.x);
        // cm.add_elevation(-0.003f * cursor_delta.y);
        cm.set_elevation(math::utils::clamp<float>(cm.elevation() - 0.003f * cursor_delta.y, 0, M_PI));
    }
    // translate view center
    if (g_context.input_state.middle_button_pressed ||
        (g_context.input_state.left_button_pressed && g_context.input_state.ctrl_key_pressed == true)) {
        float scaling = 0.001f * cm.distance();
        float dx = scaling * cursor_delta.x;
        float dy = scaling * cursor_delta.y;
        float azimuth = cm.azimuth();
        math::Vector3f delta(-std::cos(azimuth) * dx - std::sin(azimuth) * dy,
                             -std::sin(azimuth) * dx + std::cos(azimuth) * dy, 0.0f);
        cm.translate(delta);
    }
    // zoom view
    if (g_context.input_state.right_button_pressed ||
        (g_context.input_state.left_button_pressed && g_context.input_state.shift_key_pressed == true)) {
        cm.set_distance(cm.distance() * (1 - cursor_delta.y / 100.0f));
    }
    g_context.input_state.previous_cursor_pos = cursor_pos;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse) {
        // do nothing if the mouse is on top a UI element
        return;
    }
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_context.input_state.left_button_pressed = action == GLFW_PRESS;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        g_context.input_state.middle_button_pressed = action == GLFW_PRESS;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        g_context.input_state.right_button_pressed = action == GLFW_PRESS;
    }
}

void window_size_callback(GLFWwindow *window, int width, int height) {
    g_context.window_state.window_size = {width, height};
    printf("win size: %d %d\n", width, height);
}

void framebuffer_size_callback(GLFWwindow *window, int fb_width, int fb_height) {
    g_context.window_state.framebuffer_size = {fb_width, fb_height};
    printf("fb size: %d %d\n", fb_width, fb_height);
}
}


namespace glengine {

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error (%d) creating context: %s\n", error, description);
}

// initialize the context
bool ContextGLFW::init(const Config &config) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    g_context.window = glfwCreateWindow(config.window_width, config.window_height, config.window_title.c_str(), NULL, NULL);
    if (!g_context.window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(g_context.window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(config.vsync ? 1 : 0);

    printf("[context] OpenGL version %s\n", (char *)glGetString(GL_VERSION));

    // callbacks
    glfwSetScrollCallback(g_context.window, scroll_callback);
    glfwSetMouseButtonCallback(g_context.window, mouse_button_callback);
    glfwSetKeyCallback(g_context.window, key_callback);
    glfwSetCursorPosCallback(g_context.window, cursor_position_callback);
    glfwSetCursorEnterCallback(g_context.window, nullptr);
    glfwSetCharCallback(g_context.window, nullptr);
    glfwSetWindowSizeCallback(g_context.window, window_size_callback);
    glfwSetFramebufferSizeCallback(g_context.window, framebuffer_size_callback);

    // update window info in the context
    glfwGetWindowSize(g_context.window, &g_context.window_state.window_size.x, &g_context.window_state.window_size.y);
    glfwGetFramebufferSize(g_context.window, &g_context.window_state.framebuffer_size.x, &g_context.window_state.framebuffer_size.y);
    printf("[context] win size: %d %d\n",g_context.window_state.window_size.x, g_context.window_state.window_size.y);
    printf("[context] fb size: %d %d\n",g_context.window_state.framebuffer_size.x, g_context.window_state.framebuffer_size.y);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(g_context.window, true);
    // ImGui_ImplOpenGL3_Init(NULL);

    // init sokol-gfx
    sg_setup((sg_desc){0});
    return true;
}
// destroy the context
bool ContextGLFW::destroy() {
    // Cleanup
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(g_context.window);
    glfwTerminate();
    return true;
}

void ContextGLFW::register_engine_instance(GLEngine *eng) {
    Context::register_engine_instance(eng);
    glfwSetWindowUserPointer(g_context.window, eng);
}

void ContextGLFW::begin_frame() {
    // update imgui inputs 
    ImGui_ImplGlfw_NewFrame();
}

bool ContextGLFW::end_frame() {
    glfwSwapBuffers(g_context.window);
    glfwPollEvents();
    return !glfwWindowShouldClose(g_context.window);
}

// info on window and framebuffer size
int ContextGLFW::window_width() const {
    return g_context.window_state.window_size.x;
}

int ContextGLFW::window_height() const {
    return g_context.window_state.window_size.y;
}

int ContextGLFW::framebuffer_width() const {
    return g_context.window_state.framebuffer_size.x;
}

int ContextGLFW::framebuffer_height() const {
    return g_context.window_state.framebuffer_size.y;
}

} // namespace glengine
