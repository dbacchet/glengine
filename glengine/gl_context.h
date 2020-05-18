#pragma once

#include "math/vmath.h"

#include "glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


namespace glengine {

// callbacks: see glfw docs for more info
typedef void (*GLFWscrollfun)(GLFWwindow *, double, double);
typedef void (*GLFWmousebuttonfun)(GLFWwindow *, int, int, int);
typedef void (*GLFWkeyfun)(GLFWwindow *, int, int, int, int);
typedef void (*GLFWcursorposfun)(GLFWwindow *, double, double);
typedef void (*GLFWcursorenterfun)(GLFWwindow *, int);
typedef void (*GLFWcharfun)(GLFWwindow *, unsigned int);

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

struct Context {
    GLFWwindow *window = nullptr;
    InputState input_state;
};

struct Callbacks {
    GLFWscrollfun      scroll_fun_callback          = nullptr;
    GLFWmousebuttonfun mousebutton_fun_callback     = nullptr;
    GLFWkeyfun         key_fun_callback             = nullptr;
    GLFWcursorposfun   cursorpos_fun_callback       = nullptr;
    GLFWcursorenterfun cursorenterexit_fun_callback = nullptr;
    GLFWcharfun        char_fun_callback            = nullptr;
};

/// initialize the opengl context
Context init_context(uint32_t width = 1280, uint32_t height = 720, const char *title = "sample");
/// set the window callbacks
void set_callbacks(const Context &ctx, void *user_pointer, const Callbacks &callbacks);
/// destroy the given context
void destroy_context(Context &c);

} // namespace glengine
