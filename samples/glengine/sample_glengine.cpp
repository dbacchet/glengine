#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_types.h"
#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "gl_shader.h"
#include "gl_mesh.h"
#include "gl_engine.h"
#include "gl_object.h"
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

    glengine::Config config;
    config.window_width = 1280;
    config.window_height = 720;
    config.vsync = true;
    config.show_framebuffer_texture = true;
    config.show_imgui_statistics = true;

    glengine::GLEngine eng;
    eng.init(config);
    auto &rm = eng.resource_manager();

    // create basic renderables
    glengine::Renderable grid_renderable = {rm.create_grid_mesh("grid", 50.0f, 1.0f),
                                            rm.create_material("grid_mtl", glengine::StockShader::VertexColor)};
    glengine::Renderable box0_renderable = {rm.create_box_mesh("box", {1, 1, 1}),
                                            rm.create_material("box0_mtl", glengine::StockShader::Diffuse)};
    glengine::Renderable box1_renderable = {box0_renderable.mesh, // reuse existing box0 mesh
                                            rm.create_material("box1_mtl", glengine::StockShader::Phong)};
    glengine::Renderable box2_renderable = {box0_renderable.mesh,
                                            rm.create_material("box2_mtl", glengine::StockShader::DiffuseTextured)};
    glengine::Renderable box3_renderable = {box0_renderable.mesh,
                                            rm.create_material("box3_mtl", glengine::StockShader::VertexColor)};
    glengine::Renderable box4_renderable = {rm.create_box_mesh("box_dyn", {1, 1, 1}),
                                            rm.create_material("box3_mtl", glengine::StockShader::VertexColor)};
    glengine::Renderable axis_renderable = {rm.create_axis_mesh("axis"),
                                            rm.create_material("axis_mtl", glengine::StockShader::VertexColor)};
    glengine::Renderable sphere_renderable = {rm.create_sphere_mesh("sphere", 0.7),
                                              rm.create_material("sphere_mtl", glengine::StockShader::Phong)};
    // create custom renderables:
    glengine::Mesh *polyline_mesh = rm.create_mesh("polyline");
    polyline_mesh->init(create_polyline(), GL_LINES);
    glengine::Renderable polyline_renderable = {polyline_mesh,
                                                rm.create_material("polyline_mtl", glengine::StockShader::Flat)};
    glengine::Mesh *triangle_mesh = rm.create_mesh("triangle");
    triangle_mesh->init(triangle_vertices, GL_TRIANGLES);
    glengine::Renderable triangle_renderable = {triangle_mesh,
                                                rm.create_material("triangle_mtl", glengine::StockShader::VertexColor)};

    // add renderables to the scene
    auto &grid = *eng.create_renderobject(grid_renderable, nullptr, 101); // renderable is _copied_ in the renderobject
    auto &polyline = *eng.create_renderobject(polyline_renderable, nullptr, 102);
    auto &triangle = *eng.create_renderobject(triangle_renderable, nullptr, 103);
    auto &box0 = *eng.create_renderobject(box0_renderable, nullptr, 104);
    auto &box1 = *eng.create_renderobject(box1_renderable, nullptr, 105);
    auto &box2 = *eng.create_renderobject(box2_renderable, nullptr, 106);
    auto &box3 = *eng.create_renderobject(box3_renderable, nullptr, 107);
    auto &box4 = *eng.create_renderobject(box4_renderable, nullptr, 108);
    auto &axis = *eng.create_renderobject(axis_renderable, nullptr, 109);
    auto &sphere = *eng.create_renderobject(sphere_renderable, nullptr, 110);

    // change object attributes
    box2_renderable.material->_textures[(uint8_t)glengine::Material::TextureType::BaseColor] =
        rm.create_texture_from_file("uv_grid_256.png");

    // basic object hierarchy
    auto sg0 = eng.create_renderobject(
        {box0_renderable.mesh, rm.create_material("mtl", glengine::StockShader::Diffuse)}, nullptr, 120);
    auto sg1 = eng.create_renderobject(
        {box0_renderable.mesh, rm.create_material("mtl", glengine::StockShader::Diffuse)}, sg0, 121);
    auto sg2 = eng.create_renderobject(
        {box0_renderable.mesh, rm.create_material("mtl", glengine::StockShader::Diffuse)}, sg1, 122);
    auto sg3 = eng.create_renderobject(
        {box0_renderable.mesh, rm.create_material("mtl", glengine::StockShader::Diffuse)}, sg1, 123);
    sg0->set_transform(math::create_transformation<float>({1, 0, 0}, math::quat_from_euler_321<float>(0, 0, 0.5)))
        .set_scale({0.7f, 0.7f, 0.7f});
    sg1->set_transform(math::create_transformation<float>({2, 0, 0}, math::quat_from_euler_321<float>(0, 0, 0)))
        .set_scale({0.5f, 0.5f, 0.5f});
    sg2->set_transform(math::create_transformation<float>({2, 1, 0}, math::quat_from_euler_321<float>(0.3, 0, 0)));
    sg2->_renderables[0].material->base_color_factor = {1.0f, 0.0f, 1.0f, 1.0f};
    sg3->set_transform(math::create_transformation<float>({2, -1, 0}, math::quat_from_euler_321<float>(0.7, 0, 0)));
    sg3->_renderables[0].material->base_color_factor = {0.0f, 1.0f, 0.0f, 1.0f};
    // explicitly create and add another object to the hierarchy
    glengine::Renderable sg4_renderable = {rm.create_box_mesh("box_rnd_mesh", {2, 2, 2}),
                                           rm.create_material("box_rnd_mtl", glengine::StockShader::Diffuse)};
    sg4_renderable.material->base_color_factor = {1.0f, 0.0f, 0.0f, 1.0f};
    auto sg4 = eng.create_renderobject(sg3, 124);
    sg4->add_renderable(&sg4_renderable, 1);
    sg4->set_transform(math::create_transformation<float>({2, -1, -3}, math::quat_from_euler_321<float>(0.7, 0, 0)));

    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f);

    eng.add_ui_function([&]() {
        ImGui::Begin("Object Info");
        auto id = eng.object_at_screen_coord(eng.cursor_pos());
        ImGui::Text("Object id: %d", id);
        ImGui::End();
    });

    (void)grid; // unused var

    int cnt = 0;
    while (eng.render()) {
        float t = glfwGetTime();
        float k1 = float(std::sin(cnt/100.0f)+1)/2;
        float k2 = float(std::cos(cnt/100.0f)+1)/2;

        sg4_renderable.material->base_color_factor = {k1, 0.0f, k2, 1.0f};

        // polyline
        polyline_renderable.material->base_color_factor = {k2, 0.0f, k1, 1.0f};
        // triangle
        triangle .set_transform( math::create_transformation({0.0f, 0.0f, 0.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)))
                 .set_scale({1.0f + 0.5f * std::sin(2 * t), 1.0f, 1.0f});
        // box (static)
        box0.set_transform( math::create_transformation({-1.0f, 0.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)));
        box0_renderable.material->base_color_factor = {k1, k2, 0.0f, 1.0f};
        // another box using the same mesh
        box1.set_transform( math::create_transformation({1.0f, 0.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)));
        box1_renderable.material->base_color_factor = {k1, 0.0f, k2, 1.0f};
        // third box
        box2.set_transform( math::create_transformation({2.0f, 0.0f, 0.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)))
            .set_scale({1.5f, 1.5f, 1.5f});
        // fourth box
        box3.set_transform( math::create_transformation({-2.0f, -1.5f, 0.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)))
            .set_scale({0.5f, 0.5f, 0.5f});
        // modify box4 (update and draw)
        auto &bm = *box4._renderables[0].mesh;
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
        box4.set_transform( math::create_transformation({0.0f, 0.0f, 3.0f}, math::quat_from_euler_321(0.0f, 0.0f, 0.0f)));
        // axis
        axis.set_transform( math::create_transformation({0.0f, -1.0f, 2.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 2.1f)));
        // sphere
        sphere.set_transform( math::create_transformation({0.0f, 3.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, 0.0f)));
        sphere_renderable.material->base_color_factor = {0.0f, k1, k2, 1.0f};

        // scenegraph
        sg0->set_transform( math::create_transformation({2.0f, 0.0f, -1.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)));
        sg1->set_transform(math::create_transformation({2.0f + std::sin(t), 0.0f, 0.0f}, math::quat_from_euler_321(3 * t, 0.0f, 0.0f)))
            .set_visible((int(t * 2) % 2 == 1));

        cnt++;
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
