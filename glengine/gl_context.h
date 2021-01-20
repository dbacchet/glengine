#pragma once

#include "math/vmath.h"

#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


namespace glengine {

// generic context config parameters
struct Config {
    uint32_t window_width = 1280;
    uint32_t window_height = 720;
    bool vsync = true;
    // debug flags
    bool show_framebuffer_texture = false;
    bool show_imgui_statistics = false;
    uint16_t msaa_samples = 4;
    bool use_mrt = false;
};


// callbacks: see glfw docs for more info
typedef void (*GLFWscrollfun)(GLFWwindow *, double, double);
typedef void (*GLFWmousebuttonfun)(GLFWwindow *, int, int, int);
typedef void (*GLFWkeyfun)(GLFWwindow *, int, int, int, int);
typedef void (*GLFWcursorposfun)(GLFWwindow *, double, double);
typedef void (*GLFWcursorenterfun)(GLFWwindow *, int);
typedef void (*GLFWcharfun)(GLFWwindow *, unsigned int);
typedef void (*GLFWframebuffersizefun) (GLFWwindow *, int, int);
typedef void (*GLFWwindowsizefun) (GLFWwindow *, int, int);

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

struct Context {
    GLFWwindow *window = nullptr;
    InputState input_state;
    WindowState window_state;
};

struct Callbacks {
    GLFWscrollfun          scroll_fun_callback          = nullptr;
    GLFWmousebuttonfun     mousebutton_fun_callback     = nullptr;
    GLFWkeyfun             key_fun_callback             = nullptr;
    GLFWcursorposfun       cursorpos_fun_callback       = nullptr;
    GLFWcursorenterfun     cursorenterexit_fun_callback = nullptr;
    GLFWcharfun            char_fun_callback            = nullptr;
    GLFWwindowsizefun      window_size_callback         = nullptr;
    GLFWframebuffersizefun framebuffer_size_callback    = nullptr;
};

/// initialize the opengl context
Context init_context(const Config &config = {}, const char *title = "sample", void *user_pointer=nullptr, const Callbacks &callbacks={});
/// set the window callbacks
void set_callbacks(const Context &ctx, void *user_pointer, const Callbacks &callbacks);
/// destroy the given context
void destroy_context(Context &c);

} // namespace glengine
