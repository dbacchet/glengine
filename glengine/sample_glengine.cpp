#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_types.h"
#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "gl_shader.h"
#include "gl_mesh.h"
#include "gl_engine.h"

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

std::vector<glengine::Vertex> triangle_vertices = {{{-0.6f, -0.4f, 0.0f}, {255, 0, 0, 255}},
                                                   {{0.6f, -0.4f, 0.0f}, {0, 255, 0, 255}},
                                                   {{0.0f, 0.6f, 0.0f}, {0, 0, 255, 255}}};

bool create_box(const math::Vector3f &size, std::vector<glengine::Vertex> &out_vertices, std::vector<uint32_t> &out_indices) {
    float a = size.x/2.0f;
    float b = size.y/2.0f;
    float c = size.z/2.0f;
    std::vector<glengine::Vertex> vertices = { {{-a,-b, c}, {50,50,250,255}},   // top
                                               {{ a,-b, c}, {50,50,250,255}},
                                               {{ a, b, c}, {50,50,250,255}},
                                               {{-a, b, c}, {50,50,250,255}},
                                               {{-a,-b,-c}, {50,50,250,255}},   // bottom
                                               {{ a,-b,-c}, {50,50,250,255}},
                                               {{ a, b,-c}, {50,50,250,255}},
                                               {{-a, b,-c}, {50,50,250,255}},
                                               {{ a,-b,-c}, {250,50,50,255}},   // front
                                               {{ a, b,-c}, {250,50,50,255}},
                                               {{ a, b, c}, {250,50,50,255}},
                                               {{ a,-b, c}, {250,50,50,255}},
                                               {{-a,-b,-c}, {250,50,50,255}},   // back
                                               {{-a, b,-c}, {250,50,50,255}},
                                               {{-a, b, c}, {250,50,50,255}},
                                               {{-a,-b, c}, {250,50,50,255}},
                                               {{ a, b,-c}, {50,250,50,255}},   // left
                                               {{-a, b,-c}, {50,250,50,255}},
                                               {{-a, b, c}, {50,250,50,255}},
                                               {{ a, b, c}, {50,250,50,255}},
                                               {{-a,-b,-c}, {50,250,50,255}},   // right
                                               {{ a,-b,-c}, {50,250,50,255}},
                                               {{ a,-b, c}, {50,250,50,255}},
                                               {{-a,-b, c}, {50,250,50,255}},
    };
    std::vector<uint32_t> indices = {0,1,2, 0,2,3, 
                                     4,7,6, 4,6,5,
                                     8,9,10, 8,10,11,
                                     12,15,14, 12,14,13,
                                     16,17,18, 16,18,19,
                                     20,21,22, 20,22,23
    };
    out_vertices = vertices;
    out_indices = indices;
    return true;
}

std::vector<glengine::Vertex> create_grid(float len, float step) {
    std::vector<glengine::Vertex> vertices;
    const int32_t major = 5;
    int32_t nlines = len / step;
    glengine::Color col_x = {120, 20, 20, 255};
    glengine::Color col_y = {20, 120, 20, 255};
    glengine::Color col1 = {40, 40, 40, 255};
    glengine::Color col2 = {70, 70, 70, 255};
    // main axis
    vertices.push_back({{-len, 0, 0}, col_x});
    vertices.push_back({{len, 0, 0}, col_x});
    vertices.push_back({{0, -len, 0}, col_y});
    vertices.push_back({{0, len, 0}, col_y});
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
std::vector<glengine::Vertex> grid_vertices = create_grid(20, 1);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void scroll_callback(GLFWwindow *window, double xoffs, double yoffs) {
    auto &cam_manip = *(glengine::CameraManipulator *)glfwGetWindowUserPointer(window);
    cam_manip.set_distance(cam_manip.distance() * (1 + yoffs / 10));
}

int main(void) {

    glengine::GLEngine eng;
    eng.init({1280, 720, true});

    std::vector<glengine::Vertex> box_vertices;
    std::vector<uint32_t> box_indices;
    create_box({1,1,1}, box_vertices, box_indices);

    // shader
    glengine::Shader &shader = eng.create_shader(1);
    shader.init(vertex_shader_text.c_str(), fragment_shader_text.c_str());

    // meshes
    glengine::Mesh &triangle = eng.create_mesh(1);
    glengine::Mesh &box = eng.create_mesh(2);
    glengine::Mesh &grid = eng.create_mesh(3);
    triangle.init(triangle_vertices, GL_TRIANGLES);
    box.init(box_vertices, box_indices, GL_TRIANGLES);
    grid.init(grid_vertices, GL_LINES);

    glUseProgram(shader.program_id);
    GLint mvp_location = glGetUniformLocation(shader.program_id, "MVP");

    eng._camera.set_perspective(0.1, 100.0, math::utils::deg2rad(45.0f));
    // eng._camera,set_ortho(-100, 100, -20, 20, -10, 10);
    eng._camera.set_transform(
        math::create_lookat<float>({-10.0f, -1.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));
    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST); 

    int width = 0, height = 0, cnt = 0;
    while (!glfwWindowShouldClose(eng._context.window)) {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        float t = glfwGetTime();
        glfwGetFramebufferSize(eng._context.window, &width, &height);

        eng._camera_manipulator.update(eng._camera);
        eng._camera.update(width, height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        math::Matrix4f pv = eng._camera.projection() * eng._camera.inverse_transform();

        // grid
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, pv);
        grid.draw(shader.program_id);
        // triangle
        math::Matrix4f m =
            math::create_transformation({0.0f, 0.0f, 0.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f));
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, pv * m);
        triangle.draw(shader.program_id);
        // box (update and draw)
        box.vertices[0]  = {{-0.5f+0.1f*std::cos(3*t),-0.5f+0.1f*std::sin(3*t),0.5f},{50,50,200,255}};
        box.vertices[15] = {{-0.5f+0.1f*std::cos(3*t),-0.5f+0.1f*std::sin(3*t),0.5f},{50,50,200,255}};
        box.vertices[23] = {{-0.5f+0.1f*std::cos(3*t),-0.5f+0.1f*std::sin(3*t),0.5f},{50,50,200,255}};
        if (cnt%100==0) {
            float zoffs = cnt/1000.0f;
            box.indices.push_back(box.vertices.size());
            box.vertices.push_back({{-1.0f,-1.0f,zoffs},{100,100,100,255}});
            box.indices.push_back(box.vertices.size());
            box.vertices.push_back({{-2.0f,-1.0f,zoffs},{100,100,100,255}});
            box.indices.push_back(box.vertices.size());
            box.vertices.push_back({{-2.0f, 0.0f,zoffs},{100,100,100,255}});
        }
        box.update();
        m = math::create_transformation({0.0f, 0.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, pv*m);
        box.draw(shader.program_id);
        // another box using the same mesh
        m = math::create_transformation({2.0f, 0.0f, 0.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f));
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, pv*m);
        box.draw(shader.program_id);

        glfwSwapBuffers(eng._context.window);
        glfwPollEvents();
        cnt++;
    }

    // glengine::destroy_context(context);
    eng.terminate();

    exit(EXIT_SUCCESS);
}
