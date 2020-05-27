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
#include "gl_texture.h"

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
    auto &rm = eng.resource_manager();

    // meshes
    glengine::Mesh *grid_mesh = rm.create_grid_mesh(50.0f, 1.0f);
    glengine::Mesh *box_mesh = rm.create_box_mesh({1, 1, 1});
    glengine::Mesh *box_dyn_mesh = rm.create_box_mesh({1, 1, 1});
    glengine::Mesh *axis_mesh = rm.create_axis_mesh();
    glengine::Mesh *sphere_mesh = rm.create_sphere_mesh(0.7);
    glengine::Mesh *polyline_mesh = rm.create_mesh();
    polyline_mesh->init(create_polyline(), GL_LINES);
    glengine::Mesh *triangle_mesh = rm.create_mesh();
    triangle_mesh->init(triangle_vertices, GL_TRIANGLES);

    // render objects
    auto &grid     = *eng.create_renderobject(101, grid_mesh,     rm.get_stock_shader(glengine::StockShader::VertexColor));
    auto &polyline = *eng.create_renderobject(102, polyline_mesh, rm.get_stock_shader(glengine::StockShader::Flat));
    auto &triangle = *eng.create_renderobject(103, triangle_mesh, rm.get_stock_shader(glengine::StockShader::VertexColor));
    auto &box0     = *eng.create_renderobject(105, box_mesh,      rm.get_stock_shader(glengine::StockShader::Diffuse));
    auto &box1     = *eng.create_renderobject(106, box_mesh,      rm.get_stock_shader(glengine::StockShader::Phong));
    auto &box2     = *eng.create_renderobject(107, box_mesh,      rm.get_stock_shader(glengine::StockShader::DiffuseTextured));
    auto &box3     = *eng.create_renderobject(108, box_mesh,      rm.get_stock_shader(glengine::StockShader::VertexColor));
    auto &box_dyn  = *eng.create_renderobject(109, box_dyn_mesh,  rm.get_stock_shader(glengine::StockShader::VertexColor));
    auto &axis     = *eng.create_renderobject(110, axis_mesh,     rm.get_stock_shader(glengine::StockShader::VertexColor));
    auto &sphere   = *eng.create_renderobject(111, sphere_mesh,   rm.get_stock_shader(glengine::StockShader::Phong));

    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f);

    // load texture
    // box_mesh->textures.diffuse = rm.create_texture_from_file("uv_grid_256.png");
    box_mesh->textures.diffuse = rm.create_texture_from_file("Material_109_baseColor.png");

    eng.add_ui_function([&](){
            ImGui::Begin("Object Info");
            auto id = eng.object_at_screen_coord(eng.cursor_pos());
            ImGui::Text("Object id: %d", id);
            ImGui::End();
            });

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
            .set_scale({1.5f, 1.5f, 1.5f});
        // fourth box
        box3.set_transform(math::create_transformation({-2.0f, -1.5f, 0.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)))
            .set_scale({0.5f, 0.5f, 0.5f});
        // box (update and draw)
        auto &bm = *box_dyn._meshes[0];
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
        box_dyn.set_transform( math::create_transformation({0.0f, 0.0f, 3.0f}, math::quat_from_euler_321(0.0f, 0.0f, 0.0f)));
        // axis
        axis.set_transform( math::create_transformation({0.0f,-1.0f, 2.0f}, math::quat_from_euler_321(0.0f, 0.0f, t*2.1f)));
        // sphere
        sphere.set_transform( math::create_transformation({0.0f, 3.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, 0.0f)))
            .set_color({0, k1, k2, 255});

        cnt++;
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
