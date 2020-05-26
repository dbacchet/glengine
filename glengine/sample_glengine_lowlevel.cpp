#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_types.h"
#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "gl_shader.h"
#include "gl_mesh.h"
#include "gl_engine.h"
#include "gl_renderobject.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>



std::vector<glengine::Vertex> triangle_vertices = {{{-0.6f, -0.4f, 0.0f}, {255, 0, 0, 255}},
                                                   {{0.6f, -0.4f, 0.0f}, {0, 255, 0, 255}},
                                                   {{0.0f, 0.6f, 0.0f}, {0, 0, 255, 255}}};

bool create_box(const math::Vector3f &size, std::vector<glengine::Vertex> &out_vertices, std::vector<uint32_t> &out_indices) {
    float a = size.x/2.0f;
    float b = size.y/2.0f;
    float c = size.z/2.0f;

    std::vector<glengine::Vertex> vertices = { {{-a,-b, c}, {50,50,250,255}, { 0, 0, 1}, {0,0}},   // top
                                               {{ a,-b, c}, {50,50,250,255}, { 0, 0, 1}, {0,0}},
                                               {{ a, b, c}, {50,50,250,255}, { 0, 0, 1}, {0,0}},
                                               {{-a, b, c}, {50,50,250,255}, { 0, 0, 1}, {0,0}},
                                               {{-a,-b,-c}, {50,50,250,255}, { 0, 0,-1}, {0,0}},   // bottom
                                               {{ a,-b,-c}, {50,50,250,255}, { 0, 0,-1}, {0,0}},
                                               {{ a, b,-c}, {50,50,250,255}, { 0, 0,-1}, {0,0}},
                                               {{-a, b,-c}, {50,50,250,255}, { 0, 0,-1}, {0,0}},
                                               {{ a,-b,-c}, {250,50,50,255}, { 1, 0, 0}, {0,0}},   // front
                                               {{ a, b,-c}, {250,50,50,255}, { 1, 0, 0}, {0,0}},
                                               {{ a, b, c}, {250,50,50,255}, { 1, 0, 0}, {0,0}},
                                               {{ a,-b, c}, {250,50,50,255}, { 1, 0, 0}, {0,0}},
                                               {{-a,-b,-c}, {250,50,50,255}, {-1, 0, 0}, {0,0}},   // back
                                               {{-a, b,-c}, {250,50,50,255}, {-1, 0, 0}, {0,0}},
                                               {{-a, b, c}, {250,50,50,255}, {-1, 0, 0}, {0,0}},
                                               {{-a,-b, c}, {250,50,50,255}, {-1, 0, 0}, {0,0}},
                                               {{ a, b,-c}, {50,250,50,255}, { 0, 1, 0}, {0,0}},   // left
                                               {{-a, b,-c}, {50,250,50,255}, { 0, 1, 0}, {0,0}},
                                               {{-a, b, c}, {50,250,50,255}, { 0, 1, 0}, {0,0}},
                                               {{ a, b, c}, {50,250,50,255}, { 0, 1, 0}, {0,0}},
                                               {{-a,-b,-c}, {50,250,50,255}, { 0,-1, 0}, {0,0}},   // right
                                               {{ a,-b,-c}, {50,250,50,255}, { 0,-1, 0}, {0,0}},
                                               {{ a,-b, c}, {50,250,50,255}, { 0,-1, 0}, {0,0}},
                                               {{-a,-b, c}, {50,250,50,255}, { 0,-1, 0}, {0,0}},
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

std::vector<glengine::Vertex> create_polyline() {
    std::vector<glengine::Vertex> vertices;
    float r = 3.0f;
    for (float s=0; s<15.0f; s+=0.1) {
        vertices.push_back({{r*std::cos(s), r*std::sin(s), s/3.0f}});
    }
    return vertices;
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


int main(void) {

    glengine::GLEngine eng;
    eng.init({1280, 720, true});

    std::vector<glengine::Vertex> box_vertices;
    std::vector<uint32_t> box_indices;
    create_box({1,1,1}, box_vertices, box_indices);

    // shader
    glengine::ShaderSrc flat_src = glengine::get_stock_shader_source(glengine::StockShader::Flat);
    glengine::ShaderSrc diffuse_src = glengine::get_stock_shader_source(glengine::StockShader::Diffuse);
    glengine::ShaderSrc phong_src = glengine::get_stock_shader_source(glengine::StockShader::Phong);
    glengine::ShaderSrc vertexcolor_src = glengine::get_stock_shader_source(glengine::StockShader::VertexColor);
    glengine::Shader *shader_flat = eng.resource_manager().create_shader();
    glengine::Shader *shader_diffuse = eng.resource_manager().create_shader();
    glengine::Shader *shader_phong = eng.resource_manager().create_shader();
    glengine::Shader *shader_vertexcolor = eng.resource_manager().create_shader();
    shader_flat->init(flat_src.vertex_shader, flat_src.fragment_shader);
    shader_diffuse->init(diffuse_src.vertex_shader, diffuse_src.fragment_shader);
    shader_phong->init(phong_src.vertex_shader, phong_src.fragment_shader);
    shader_vertexcolor->init(vertexcolor_src.vertex_shader, vertexcolor_src.fragment_shader);

    // meshes
    glengine::Mesh *grid_mesh = eng.create_mesh();
    glengine::Mesh *polyline_mesh = eng.create_mesh();
    glengine::Mesh *triangle_mesh = eng.create_mesh();
    glengine::Mesh *box_mesh = eng.create_mesh();
    glengine::Mesh *box_dyn_mesh = eng.create_mesh();
    grid_mesh->init(grid_vertices, GL_LINES);
    polyline_mesh->init(create_polyline(), GL_LINES);
    triangle_mesh->init(triangle_vertices, GL_TRIANGLES);
    box_mesh->init(box_vertices, box_indices, GL_TRIANGLES);
    box_dyn_mesh->init(box_vertices, box_indices, GL_TRIANGLES);

    // render objects
    glengine::RenderObject grid;
    grid.init(grid_mesh, shader_vertexcolor);
    glengine::RenderObject polyline;
    polyline.init(polyline_mesh, shader_flat);
    glengine::RenderObject triangle;
    triangle.init(triangle_mesh, shader_vertexcolor);
    glengine::RenderObject box0;
    box0.init(box_mesh, shader_diffuse);
    glengine::RenderObject box1;
    box1.init(box_mesh, shader_phong);
    glengine::RenderObject box2;
    box2.init(box_mesh, shader_vertexcolor);
    glengine::RenderObject box3;
    box3.init(box_mesh, shader_phong);
    glengine::RenderObject box_dyn; // we'll deform the vertices on this one
    box_dyn.init(box_dyn_mesh, shader_vertexcolor);

    eng._camera.set_perspective(0.1, 100.0, math::utils::deg2rad(45.0f));
    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    int width = 0, height = 0, cnt = 0;
    while (!glfwWindowShouldClose(eng._context.window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float t = glfwGetTime();
        glfwGetFramebufferSize(eng._context.window, &width, &height);

        eng._camera_manipulator.update(eng._camera);
        eng._camera.update(width, height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        math::Matrix4f pv = eng._camera.projection() * eng._camera.inverse_transform();

        // grid
        grid.draw(eng._camera);
        // polyline
        polyline.set_color({255,0,0,255}).draw(eng._camera);
        // triangle
        triangle
            .set_transform( math::create_transformation({0.0f, 0.0f, 0.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)))
            .set_scale({1.0f + 0.5f * std::sin(2 * t), 1.0f, 1.0f});
        triangle.draw(eng._camera);
        // box (static)
        box0.set_transform(math::create_transformation({-1.0f, 0.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)))
            .set_color({uint8_t(cnt%255),255,0,255});
        box0.draw(eng._camera);
        // another box using the same mesh
        box1.set_transform(math::create_transformation({ 1.0f, 0.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)))
            .set_color({uint8_t(cnt%255),0,255,255});
        box1.draw(eng._camera);
        // third box
        box2.set_transform(math::create_transformation({2.0f, 0.0f, 0.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)))
            .set_scale({0.5f,0.5f,0.5f});
        box2.draw(eng._camera);
        // fourth box
        box3.set_transform(math::create_transformation({-2.0f,-1.5f, 0.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)))
            .set_scale({0.5f,0.5f,0.5f});
        box3.draw(eng._camera);
        // box (update and draw)
        auto &bm = *(box_dyn._meshes[0]);
        bm.vertices[0]  = {{-0.5f+0.2f*std::cos(3*t),-0.5f+0.2f*std::sin(3*t),0.5f},{50,50,200,255}};
        bm.vertices[15] = {{-0.5f+0.2f*std::cos(3*t),-0.5f+0.2f*std::sin(3*t),0.5f},{50,50,200,255}};
        bm.vertices[23] = {{-0.5f+0.2f*std::cos(3*t),-0.5f+0.2f*std::sin(3*t),0.5f},{50,50,200,255}};
        if (cnt%100==0) {
            float zoffs = cnt/1000.0f;
            bm.indices.push_back(bm.vertices.size());
            bm.vertices.push_back({{-1.0f,-1.0f,zoffs},{100,100,100,255}});
            bm.indices.push_back(bm.vertices.size());
            bm.vertices.push_back({{-2.0f,-1.0f,zoffs},{100,100,100,255}});
            bm.indices.push_back(bm.vertices.size());
            bm.vertices.push_back({{-2.0f, 0.0f,zoffs},{100,100,100,255}});
        }
        bm.update();
        box_dyn.set_transform(math::create_transformation({0.0f, 0.0f, 3.0f}, math::quat_from_euler_321(0.0f, 0.0f, 0.0f)));
        box_dyn.draw(eng._camera);

        glfwSwapBuffers(eng._context.window);
        glfwPollEvents();
        cnt++;
    }

    // glengine::destroy_context(context);
    eng.terminate();

    exit(EXIT_SUCCESS);
}
