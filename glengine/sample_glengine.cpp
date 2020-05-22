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

std::vector<glengine::Vertex> create_polyline() {
    std::vector<glengine::Vertex> vertices;
    float r = 3.0f;
    for (float s = 0; s < 15.0f; s += 0.1) {
        vertices.push_back({{r * std::cos(s), r * std::sin(s), s / 3.0f}});
    }
    return vertices;
}

int main(void) {

    glengine::GLEngine eng;
    eng.init({1280, 720, true});

    // meshes
    glengine::Mesh *grid_mesh = eng.create_grid_mesh(204, 50.0f, 1.0f);
    glengine::Mesh *box_mesh = eng.create_box_mesh(202, {1, 1, 1});
    glengine::Mesh *box_dyn_mesh = eng.create_box_mesh(203, {1, 1, 1});
    glengine::Mesh *polyline_mesh = eng.create_mesh(200);
    polyline_mesh->init(create_polyline(), GL_LINES);
    glengine::Mesh *triangle_mesh = eng.create_mesh(201);
    triangle_mesh->init(triangle_vertices, GL_TRIANGLES);

    // render objects
    auto &grid     = *eng.create_renderobject(101, grid_mesh,     eng.get_stock_shader(glengine::StockShader::VertexColor));
    auto &polyline = *eng.create_renderobject(102, polyline_mesh, eng.get_stock_shader(glengine::StockShader::Flat));
    auto &triangle = *eng.create_renderobject(103, triangle_mesh, eng.get_stock_shader(glengine::StockShader::VertexColor));
    auto &box0     = *eng.create_renderobject(105, box_mesh,      eng.get_stock_shader(glengine::StockShader::Diffuse));
    auto &box1     = *eng.create_renderobject(106, box_mesh,      eng.get_stock_shader(glengine::StockShader::Phong));
    auto &box2     = *eng.create_renderobject(107, box_mesh,      eng.get_stock_shader(glengine::StockShader::VertexColor));
    auto &box3     = *eng.create_renderobject(108, box_mesh,      eng.get_stock_shader(glengine::StockShader::Phong));
    auto &box_dyn  = *eng.create_renderobject(109, box_dyn_mesh,  eng.get_stock_shader(glengine::StockShader::VertexColor));

    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    int width = 0, height = 0, cnt = 0;
    while (eng.render()) {
        float t = glfwGetTime();
        uint8_t k1 = uint8_t(cnt % 255);
        uint8_t k2 = uint8_t(255 - cnt % 255);

        // polyline
        polyline.set_color({k2, 0, k1, 255});
        // triangle
        triangle
            .set_transform(math::create_transformation({0.0f, 0.0f, 0.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)))
            .set_scale({1.0f + 0.5f * std::sin(2 * t), 1.0f, 1.0f});
        // box (static)
        box0.set_transform(math::create_transformation({-1.0f, 0.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)))
            .set_color({k1, k2, 0, 255});
        // another box using the same mesh
        box1.set_transform(math::create_transformation({1.0f, 0.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)))
            .set_color({k1, 0, k2, 255});
        // third box
        box2.set_transform(math::create_transformation({2.0f, 0.0f, 0.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)))
            .set_scale({0.5f, 0.5f, 0.5f});
        // fourth box
        box3.set_transform(math::create_transformation({-2.0f, -1.5f, 0.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)))
            .set_scale({0.5f, 0.5f, 0.5f});
        // box (update and draw)
        auto &bm = *box_dyn._mesh;
        bm.vertices[0] = {{-0.5f + 0.2f * std::cos(3 * t), -0.5f + 0.2f * std::sin(3 * t), 0.5f}, {50, 50, 200, 255}};
        bm.vertices[15] = {{-0.5f + 0.2f * std::cos(3 * t), -0.5f + 0.2f * std::sin(3 * t), 0.5f}, {50, 50, 200, 255}};
        bm.vertices[23] = {{-0.5f + 0.2f * std::cos(3 * t), -0.5f + 0.2f * std::sin(3 * t), 0.5f}, {50, 50, 200, 255}};
        if (cnt % 100 == 0) {
            float zoffs = cnt / 1000.0f;
            bm.indices.push_back(bm.vertices.size());
            bm.vertices.push_back({{-1.0f, -1.0f, zoffs}, {100, 100, 100, 255}});
            bm.indices.push_back(bm.vertices.size());
            bm.vertices.push_back({{-2.0f, -1.0f, zoffs}, {100, 100, 100, 255}});
            bm.indices.push_back(bm.vertices.size());
            bm.vertices.push_back({{-2.0f, 0.0f, zoffs}, {100, 100, 100, 255}});
        }
        bm.update();
        box_dyn.set_transform(
            math::create_transformation({0.0f, 0.0f, 3.0f}, math::quat_from_euler_321(0.0f, 0.0f, 0.0f)));

        cnt++;
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
