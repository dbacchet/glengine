#include "gl_engine.h"
#include "gl_renderobject.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

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
} // namespace

namespace glengine {

GLEngine::~GLEngine() {}

bool GLEngine::init(const Config &config) {
    _context = glengine::init_context(config, "GLEngine sample app", (void *)this,
                                      {
                                          scroll_callback,          // scroll_fun_callback
                                          mouse_button_callback,    // mousebutton_fun_callback
                                          key_callback,             // key_fun_callback
                                          cursor_position_callback, // cursorpos_fun_callback
                                          nullptr,                  // cursorenterexit_fun_callback
                                          nullptr,                  // char_fun_callback
                                      });

    _camera.set_perspective(0.1, 1000.0, math::utils::deg2rad(45.0f));
    _camera.set_transform(math::create_lookat<float>({-10.0f, -1.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));

    // create stock shaders and prefabs
    create_stock_shaders();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    return true;
}

bool GLEngine::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width = -1;
    int height = -1;
    glfwGetFramebufferSize(_context.window, &width, &height);

    _camera_manipulator.update(_camera);
    _camera.update(width, height);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto &ro : _renderobjects) {
        ro.second->draw(_camera);
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowMetricsWindow();

    for (auto& fun: _ui_functions) {
        fun();
    }

    // render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(_context.window);
    glfwPollEvents();
    return !glfwWindowShouldClose(_context.window);
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

Mesh *GLEngine::create_mesh(uint32_t id) {
    Mesh *m = new Mesh();
    _meshes[id] = m;
    return m;
}

Mesh *GLEngine::get_mesh(uint32_t id) {
    return _meshes[id];
}

bool GLEngine::has_mesh(uint32_t id) const {
    return _meshes.count(id) > 0;
}

Mesh *GLEngine::create_axis_mesh(uint32_t id) {
    Mesh *m = create_mesh(id);
    MeshData md = create_axis_data();
    m->init(md.vertices, md.indices, GL_LINES);
    return m;
}

Mesh *GLEngine::create_box_mesh(uint32_t id, const math::Vector3f &size) {
    Mesh *m = create_mesh(id);
    MeshData md = create_box_data(size);
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *GLEngine::create_sphere_mesh(uint32_t id, float radius, uint32_t subdiv) {
    Mesh *m = create_mesh(id);
    MeshData md = create_sphere_data(radius, subdiv);
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *GLEngine::create_grid_mesh(uint32_t id, float len, float step) {
    Mesh *m = create_mesh(id);
    MeshData md = create_grid_data(len, step);
    m->init(md.vertices, md.indices, GL_LINES);
    return m;
}

Shader *GLEngine::create_shader(uint32_t id) {
    Shader *s = new Shader();
    _shaders[id] = s;
    return s;
}

Shader *GLEngine::get_shader(uint32_t id) {
    return _shaders[id];
}

bool GLEngine::has_shader(uint32_t id) const {
    return _shaders.count(id) > 0;
}

Shader *GLEngine::get_stock_shader(StockShader type) {
    return _stock_shaders[type];
}

RenderObject *GLEngine::create_renderobject(uint32_t id) {
    RenderObject *ro = new RenderObject();
    _renderobjects[id] = ro;
    return ro;
}

RenderObject *GLEngine::create_renderobject(uint32_t id, Mesh *mesh, Shader *shader) {
    RenderObject *ro = create_renderobject(id);
    ro->init(mesh, shader);
    return ro;
}

RenderObject *GLEngine::get_renderobject(uint32_t id) {
    return _renderobjects[id];
}

bool GLEngine::has_renderobject(uint32_t id) const {
    return _renderobjects.count(id) > 0;
}

// RenderObject* GLEngine::create_box(uint32_t id, const math::Vector3f &size, StockShader shader) {
//     RenderObject *ro = create_renderobject(id);
//     ro->init()
// }

void GLEngine::add_ui_function(std::function<void(void)> fun) {
    _ui_functions.push_back(fun);
}

void GLEngine::create_stock_shaders() {
    Shader *shader_flat = new Shader();
    Shader *shader_diffuse = new Shader();
    Shader *shader_phong = new Shader();
    Shader *shader_vertexcolor = new Shader();
    ShaderSrc flat_src = get_stock_shader_source(StockShader::Flat);
    ShaderSrc diffuse_src = get_stock_shader_source(StockShader::Diffuse);
    ShaderSrc phong_src = get_stock_shader_source(StockShader::Phong);
    ShaderSrc vertexcolor_src = get_stock_shader_source(StockShader::VertexColor);
    shader_flat->init(flat_src.vertex_shader, flat_src.fragment_shader);
    shader_diffuse->init(diffuse_src.vertex_shader, diffuse_src.fragment_shader);
    shader_phong->init(phong_src.vertex_shader, phong_src.fragment_shader);
    shader_vertexcolor->init(vertexcolor_src.vertex_shader, vertexcolor_src.fragment_shader);
    _stock_shaders[StockShader::Flat] = shader_flat;
    _stock_shaders[StockShader::Diffuse] = shader_diffuse;
    _stock_shaders[StockShader::Phong] = shader_phong;
    _stock_shaders[StockShader::VertexColor] = shader_vertexcolor;
}

} // namespace glengine
