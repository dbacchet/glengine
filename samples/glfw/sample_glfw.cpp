#include "glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_camera.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>

static std::string vertex_shader_text =
    R"(#version 330
uniform mat4 MVP;
layout (location=0) in vec3 vPos;
layout (location=1) in vec4 vCol;
out vec4 color;
void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    color = vCol;
})";

static std::string fragment_shader_text =
    R"(#version 330
in vec4 color;
out vec4 fragment_color;
void main()
{
    fragment_color = color;
})";

std::unordered_map<GLenum, std::string> shader_type_map = {
    {GL_FRAGMENT_SHADER, "FRAGMENT_SHADER"},
    {GL_VERTEX_SHADER, "VERTEX_SHADER"},
};

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};
struct Vertex {
    math::Vector3f pos;
    Color color;
};

class Mesh {
  public:
    // mesh data
    std::vector<Vertex> vertices;
    GLenum primitive = GL_TRIANGLES;
    GLuint shader = 0;

    Mesh(const std::vector<Vertex> &vertices_, GLenum primitive_, GLuint shader_)
    : vertices(vertices_)
    , primitive(primitive_)
    , shader(shader_) {
        setup_mesh();
    }

    void draw(GLuint shader) {
        // draw mesh
        glUseProgram(shader);
        glBindVertexArray(vao);
        glDrawArrays(primitive, 0, vertices.size());
        glBindVertexArray(0);
    }

  private:
    //  render data
    unsigned int vao, vbo;

    void setup_mesh() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glUseProgram(shader); // we can get rid of the dependency on the shader here if we stick with standard location.
                              // i.e pos is location=0, color is location=1
        // vertex positions
        GLuint vpos_location = glGetAttribLocation(shader, "vPos");
        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        // vertex color
        GLuint vcol_location = glGetAttribLocation(shader, "vCol");
        glEnableVertexAttribArray(vcol_location);
        glVertexAttribPointer(vcol_location, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, color));

        glBindVertexArray(0);
    }
};

class Shader {
  public:
    std::string _vertex_shader_source = "";
    std::string _fragment_shader_source = "";
    GLuint vertex_shader_id = 0;
    GLuint fragment_shader_id = 0;
    GLuint program_id = 0;

    Shader(const char *vs, const char *fs)
    : _vertex_shader_source(vs)
    , _fragment_shader_source(fs) {
        init();
    }

    Shader(const std::string &vs, const std::string &fs)
    : Shader(vs.c_str(), fs.c_str()) {}

    void init() {
        vertex_shader_id = create_shader(vertex_shader_text, GL_VERTEX_SHADER);
        fragment_shader_id = create_shader(fragment_shader_text, GL_FRAGMENT_SHADER);
        program_id = create_program();
    }

    GLuint create_shader(const std::string &source, GLenum shader_type) {
        GLuint shader_id = glCreateShader(shader_type);
        const char *shadersrc[1];
        shadersrc[0] = source.c_str();
        glShaderSource(shader_id, 1, shadersrc, NULL);
        glCompileShader(shader_id);
        GLint ret = 0;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &ret);
        if (ret == GL_FALSE) {
            printf("%s: error compiling shader\n", shader_type_map[shader_type].c_str());
            print_shader_info_log(shader_id);
            return 0;
        }
        return shader_id;
    }

    GLuint create_program() {
        GLuint program_id = glCreateProgram();
        glAttachShader(program_id, vertex_shader_id);
        glAttachShader(program_id, fragment_shader_id);
        glBindFragDataLocation(program_id, 0, "fragment_color");
        glLinkProgram(program_id);
        GLint ret = 0;
        glGetProgramiv(program_id, GL_LINK_STATUS, &ret);
        if (ret == GL_FALSE) {
            printf("error linking shader program\n");
            print_program_info_log(program_id);
        }
        return program_id;
    }

    void print_shader_info_log(GLuint shader) {
        int max_length = 2048;
        char log[2048];
        glGetShaderInfoLog(shader, max_length, NULL, log);
        printf("shader info log for GL index %u:\n%s", shader, log);
    }

    void print_program_info_log(GLuint program) {
        int max_length = 2048;
        char log[2048];
        glGetProgramInfoLog(program, max_length, NULL, log);
        printf("program info log for GL index %u:\n%s", program, log);
    }
};

std::vector<Vertex> vertices = {{{-0.6f,-0.4f, 0.0f}, {255, 0, 0, 255}},
                                {{ 0.6f,-0.4f, 0.0f}, {0, 255, 0, 255}},
                                {{ 0.0f, 0.6f, 0.0f}, {0, 0, 255, 255}}};

std::vector<Vertex> create_grid(float len, float step) {
    std::vector<Vertex> vertices;
    const int32_t major = 5;
    int32_t nlines = len / step;
    Color col0 = {80, 80, 80, 255};
    Color col1 = {40, 40, 40, 255};
    Color col2 = {50, 50, 50, 255};
    // main axis
    vertices.push_back({{-len, 0, 0}, col0});
    vertices.push_back({{len, 0, 0}, col0});
    vertices.push_back({{0, -len, 0}, col0});
    vertices.push_back({{0, len, 0}, col0});
    // other lines
    for (int32_t i = 1; i <= nlines; i++) {
        vertices.push_back({{-len, i * step, 0}, i % major ? col1 : col2});
        vertices.push_back({{len, i * step, 0}, i % major ? col1 : col2});
        vertices.push_back({{-len, -i * step, 0}, i % major ? col1 : col2});
        vertices.push_back({{len, -i * step, 0}, i % major ? col1 : col2});
        vertices.push_back({{i * step, -len, 0}, i % major ? col1 : col2});
        vertices.push_back({{i * step, len, 0}, i % major ? col1 : col2});
        vertices.push_back({{-i * step, -len, 0}, i % major ? col1 : col2});
        vertices.push_back({{-i * step, len, 0}, i % major ? col1 : col2});
    }
    return vertices;
}
std::vector<Vertex> grid_vertices = create_grid(20, 1);

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void) {
    GLFWwindow *window;
    // GLuint vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(1280, 720, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity
    printf("OpenGL version %s\n", (char *)glGetString(GL_VERSION));

    // shader
    Shader shader(vertex_shader_text, fragment_shader_text);

    // meshes
    Mesh triangle(vertices, GL_TRIANGLES, shader.program_id);
    Mesh grid(grid_vertices, GL_LINES, shader.program_id);

    glUseProgram(shader.program_id);
    mvp_location = glGetUniformLocation(shader.program_id, "MVP");

    GLCamera camera;
    camera.set_perspective(0.1, 100.0, math::utils::deg2rad(45.0f));
    // camera.set_ortho(-100, 100, -20, 20, -10, 10);
    camera.set_transform(math::create_lookat<float>({-10.0f, -1.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));

    int width, height;
    while (!glfwWindowShouldClose(window)) {
        float t = glfwGetTime();
        glfwGetFramebufferSize(window, &width, &height);

        camera.set_transform(math::create_lookat<float>({10.0f*std::cos(0.5f*t), 5.0f*std::sin(0.5f*t), 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));
        camera.update(width, height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        math::Matrix4f pv = camera.projection() * camera.inverse_transform();

        // grid
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, pv);
        grid.draw(shader.program_id);
        // triangle
        math::Matrix4f m = math::create_transformation({0.0f, 0.0f, 0.0f},
                                                       math::quat_from_euler_321(0.0f, 0.0f, t*1.5f));
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, pv * m);
        triangle.draw(shader.program_id);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
