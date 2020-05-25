#include "gl_engine.h"
#include "gl_renderobject.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "math/vmath.h"
#include "math/math_utils.h"

#include "stb_image_write.h"

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
    app.resize_buffers();
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
                                          window_size_callback,
                                          framebuffer_size_callback //
                                      });

    _camera.set_perspective(0.1, 1000.0, math::utils::deg2rad(45.0f));
    _camera.set_transform(math::create_lookat<float>({-10.0f, -1.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));

    // create stock shaders
    create_stock_shaders();

    // configure g-buffer framebuffer
    glGenFramebuffers(1, &_g_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _g_buffer);
    // position color buffer
    glGenTextures(1, &_gb_color);
    glBindTexture(GL_TEXTURE_2D, _gb_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gb_color, 0);
    // object id buffer
    glGenTextures(1, &_gb_id);
    glBindTexture(GL_TEXTURE_2D, _gb_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _gb_id, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // create and attach depth buffer (renderbuffer)
    glGenRenderbuffers(1, &_gb_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, _gb_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _gb_depth); // now actually attach it
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: framebuffer not complete!\n");
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // resize the cpu buffer for the object ids
    _id_buffer.resize(_context.window_state.framebuffer_size.x*_context.window_state.framebuffer_size.y, NULL_ID);

    // create the screen quad mesh
    _ss_quad = create_quad_mesh(NULL_ID);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    return true;
}

bool GLEngine::render() {

    const auto &fbsize = _context.window_state.framebuffer_size;

    _camera_manipulator.update(_camera);
    _camera.update(fbsize.x, fbsize.y);

    glViewport(0, 0, fbsize.x, fbsize.y);

    // bind to framebuffer and draw scene as we normally would to color texture 
    glBindFramebuffer(GL_FRAMEBUFFER, _g_buffer);

    glViewport(0, 0, fbsize.x, fbsize.y);
    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // "clear" the id buffer setting NULL_ID as clear value
    glClearBufferuiv(GL_COLOR, 1, &NULL_ID);

    for (auto &ro : _renderobjects) {
        ro.second->draw(_camera);
    }

    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

    glViewport(0, 0, fbsize.x, fbsize.y);
    Shader *quad_shader = get_stock_shader(StockShader::Quad);
    quad_shader->activate();
    // glUniform1i(glGetUniformLocation(quad_shader->program_id, "screen_texture"), 0); 
    glBindTexture(GL_TEXTURE_2D, _gb_color);	// use the color attachment texture as the texture of the quad plane
    _ss_quad->draw();

    // copy the id texture in cpu memory
    glBindTexture(GL_TEXTURE_2D, _gb_id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, _id_buffer.data());

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowMetricsWindow();

    for (auto& fun: _ui_functions) {
        fun();
    }

    ImGui::Begin("fb image");
    ImGui::Image((void*)(intptr_t)_gb_color, ImVec2(300,300*(float)fbsize.y/fbsize.x),ImVec2(0,1),ImVec2(1,0));
    ImGui::End();
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

Mesh *GLEngine::create_mesh(ID id) {
    Mesh *m = new Mesh();
    _meshes[id] = m;
    return m;
}

Mesh *GLEngine::get_mesh(ID id) {
    return _meshes[id];
}

bool GLEngine::has_mesh(ID id) const {
    return _meshes.count(id) > 0;
}

Mesh *GLEngine::create_axis_mesh(ID id) {
    Mesh *m = create_mesh(id);
    MeshData md = create_axis_data();
    m->init(md.vertices, md.indices, GL_LINES);
    return m;
}

Mesh *GLEngine::create_quad_mesh(ID id) {
    Mesh *m = create_mesh(id);
    MeshData md = create_quad_data();
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *GLEngine::create_box_mesh(ID id, const math::Vector3f &size) {
    Mesh *m = create_mesh(id);
    MeshData md = create_box_data(size);
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *GLEngine::create_sphere_mesh(ID id, float radius, uint32_t subdiv) {
    Mesh *m = create_mesh(id);
    MeshData md = create_sphere_data(radius, subdiv);
    m->init(md.vertices, md.indices, GL_TRIANGLES);
    return m;
}

Mesh *GLEngine::create_grid_mesh(ID id, float len, float step) {
    Mesh *m = create_mesh(id);
    MeshData md = create_grid_data(len, step);
    m->init(md.vertices, md.indices, GL_LINES);
    return m;
}

Shader *GLEngine::create_shader(ID id) {
    Shader *s = new Shader();
    _shaders[id] = s;
    return s;
}

Shader *GLEngine::get_shader(ID id) {
    return _shaders[id];
}

bool GLEngine::has_shader(ID id) const {
    return _shaders.count(id) > 0;
}

Shader *GLEngine::get_stock_shader(StockShader type) {
    return _stock_shaders[type];
}

RenderObject *GLEngine::create_renderobject(ID id) {
    RenderObject *ro = new RenderObject(id);
    _renderobjects[id] = ro;
    return ro;
}

RenderObject *GLEngine::create_renderobject(ID id, Mesh *mesh, Shader *shader) {
    RenderObject *ro = create_renderobject(id);
    ro->init(mesh, shader);
    return ro;
}

RenderObject *GLEngine::get_renderobject(ID id) {
    return _renderobjects[id];
}

bool GLEngine::has_renderobject(ID id) const {
    return _renderobjects.count(id) > 0;
}

void GLEngine::add_ui_function(std::function<void(void)> fun) {
    _ui_functions.push_back(fun);
}

math::Vector2i GLEngine::cursor_pos() const {
    return _context.input_state.previous_cursor_pos;
}

ID GLEngine::object_at_screen_coord(const math::Vector2i &cursor_pos) const {
    int32_t py = _context.window_state.framebuffer_size.y -1 - cursor_pos.y;
    uint32_t idx = py*_context.window_state.framebuffer_size.x+cursor_pos.x;
    if (idx<0 || idx>=_id_buffer.size()) {
        return NULL_ID;
    }
    return _id_buffer[idx];
}

void GLEngine::create_stock_shaders() {
    Shader *shader_flat = new Shader();
    Shader *shader_diffuse = new Shader();
    Shader *shader_phong = new Shader();
    Shader *shader_vertexcolor = new Shader();
    Shader *shader_quad = new Shader();
    ShaderSrc flat_src = get_stock_shader_source(StockShader::Flat);
    ShaderSrc diffuse_src = get_stock_shader_source(StockShader::Diffuse);
    ShaderSrc phong_src = get_stock_shader_source(StockShader::Phong);
    ShaderSrc vertexcolor_src = get_stock_shader_source(StockShader::VertexColor);
    ShaderSrc quad_src = get_stock_shader_source(StockShader::Quad);
    shader_flat->init(flat_src.vertex_shader, flat_src.fragment_shader);
    shader_diffuse->init(diffuse_src.vertex_shader, diffuse_src.fragment_shader);
    shader_phong->init(phong_src.vertex_shader, phong_src.fragment_shader);
    shader_vertexcolor->init(vertexcolor_src.vertex_shader, vertexcolor_src.fragment_shader);
    shader_quad->init(quad_src.vertex_shader, quad_src.fragment_shader);
    _stock_shaders[StockShader::Flat] = shader_flat;
    _stock_shaders[StockShader::Diffuse] = shader_diffuse;
    _stock_shaders[StockShader::Phong] = shader_phong;
    _stock_shaders[StockShader::VertexColor] = shader_vertexcolor;
    _stock_shaders[StockShader::Quad] = shader_quad;
}

void GLEngine::resize_buffers() {
    // const auto &win_size = _context.window_state.window_size;
    const auto &fb_size = _context.window_state.framebuffer_size;
    // align opengl buffers to the size of the framebuffer
    if (_gb_color!=INVALID_BUFFER) {
        glBindTexture(GL_TEXTURE_2D, _gb_color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb_size.x, fb_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    if (_gb_id!=INVALID_BUFFER) {
        glBindTexture(GL_TEXTURE_2D, _gb_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, fb_size.x, fb_size.y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    }
    if (_gb_depth!=INVALID_BUFFER) {
        glBindRenderbuffer(GL_RENDERBUFFER, _gb_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fb_size.x, fb_size.y);
    }
    _id_buffer.resize(fb_size.x*fb_size.y, NULL_ID);
}

} // namespace glengine
