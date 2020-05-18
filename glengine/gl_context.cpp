#include "gl_context.h"

#include "glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <cstdio>


namespace glengine {

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error (%d) creating context: %s\n", error, description);
}

Context init_context(uint32_t width, uint32_t height, const char *title) {

    Context context;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    context.window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!context.window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(context.window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    printf("OpenGL version %s\n", (char *)glGetString(GL_VERSION));

    return context;
}

void set_callbacks(const Context &ctx, void *user_pointer, const Callbacks &callbacks) {
    if (user_pointer) {
        glfwSetWindowUserPointer(ctx.window, user_pointer);
    }
    glfwSetScrollCallback(ctx.window, callbacks.scroll_fun_callback);
    glfwSetMouseButtonCallback(ctx.window, callbacks.mousebutton_fun_callback);
    glfwSetKeyCallback(ctx.window, callbacks.key_fun_callback);
    glfwSetCursorPosCallback(ctx.window, callbacks.cursorpos_fun_callback);
    glfwSetCursorEnterCallback(ctx.window, callbacks.cursorenterexit_fun_callback);
    glfwSetCharCallback(ctx.window, callbacks.char_fun_callback);
};

void destroy_context(Context &c) {
    glfwDestroyWindow(c.window);
    glfwTerminate();
}

} // namespace glengine
