#include "gl_context.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <cstdlib>
#include <cstdio>


namespace glengine {

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error (%d) creating context: %s\n", error, description);
}

Context init_context(const Config &config, const char *title, void *user_pointer, const Callbacks &callbacks) {

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

    context.window = glfwCreateWindow(config.window_width, config.window_height, title, NULL, NULL);
    if (!context.window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(context.window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(config.vsync ? 1 : 0);

    printf("[context] OpenGL version %s\n", (char *)glGetString(GL_VERSION));

    // callbacks
    if (user_pointer) {
        glfwSetWindowUserPointer(context.window, user_pointer);
    }
    glfwSetScrollCallback(context.window, callbacks.scroll_fun_callback);
    glfwSetMouseButtonCallback(context.window, callbacks.mousebutton_fun_callback);
    glfwSetKeyCallback(context.window, callbacks.key_fun_callback);
    glfwSetCursorPosCallback(context.window, callbacks.cursorpos_fun_callback);
    glfwSetCursorEnterCallback(context.window, callbacks.cursorenterexit_fun_callback);
    glfwSetCharCallback(context.window, callbacks.char_fun_callback);
    glfwSetWindowSizeCallback(context.window, callbacks.window_size_callback);
    glfwSetFramebufferSizeCallback(context.window, callbacks.framebuffer_size_callback);

    // update window info in the context
    glfwGetWindowSize(context.window, &context.window_state.window_size.x, &context.window_state.window_size.y);
    glfwGetFramebufferSize(context.window, &context.window_state.framebuffer_size.x, &context.window_state.framebuffer_size.y);
    printf("[context] win size: %d %d\n",context.window_state.window_size.x, context.window_state.window_size.y);
    printf("[context] fb size: %d %d\n",context.window_state.framebuffer_size.x, context.window_state.framebuffer_size.y);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsResizeFromEdges = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(context.window, true);
    ImGui_ImplOpenGL3_Init(NULL);

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
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(c.window);
    glfwTerminate();
}

} // namespace glengine
