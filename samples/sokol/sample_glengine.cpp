#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_types.h"
#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "gl_mesh.h"
#include "gl_engine.h"
#include "gl_object.h"
#include "gl_prefabs.h"
#include "gl_material_diffuse.h"
#include "gl_material_diffuse_textured.h"
#include "gl_material_flat.h"
#include "gl_material_flat_textured.h"
#include "gl_material_vertexcolor.h"

#include "imgui/imgui.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "uv_grid_256.png.h"
#include "stb/stb_image.h"

sg_image create_texture() {
    int img_width, img_height, num_channels;
    const int desired_channels = 4;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc *pixels = stbi_load_from_memory(uv_grid_256_png, (int)uv_grid_256_png_len, &img_width, &img_height,
                                            &num_channels, desired_channels);
    if (pixels) {
        sg_image img = sg_make_image((sg_image_desc){
            .width = img_width,
            .height = img_height,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .content.subimage[0][0] =
                {
                    .ptr = pixels,
                    .size = img_width * img_height * 4,
                },
            .label = "grid-256-png",
        });
        stbi_image_free(pixels);
        return img;
    }
    return {SG_INVALID_ID};
}

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
    config.show_imgui_statistics = false;

    glengine::GLEngine eng;
    eng.init(config);
    auto &rm = eng.resource_manager();

    // create basic renderables
    glengine::Renderable grid_renderable = {
        eng.create_grid_mesh(50.0f, 1.0f),
        eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)};
    glengine::Renderable box0_renderable = {
        eng.create_box_mesh({1, 1, 1}),
        eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32)};
    glengine::Renderable box1_renderable = {
        box0_renderable.mesh, // reuse existing box0 mesh
        eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES,
                                                       SG_INDEXTYPE_UINT32)}; // Phong in the original example
    glengine::Renderable box2_renderable = {
        box0_renderable.mesh,
        eng.create_material<glengine::MaterialDiffuseTextured>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32)};
    glengine::Renderable box3_renderable = {box0_renderable.mesh, eng.create_material<glengine::MaterialVertexColor>(
                                                                      SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32)};
    auto dynbox_md = glengine::create_box_data({1, 1, 1});
    glengine::Renderable box4_renderable = {
        eng.create_mesh(dynbox_md.vertices, dynbox_md.indices,
                        SG_USAGE_DYNAMIC), // dynamic mesh must be explicitly created with the full init call
        eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32)};
    glengine::Renderable axis_renderable = {eng.create_axis_mesh(), eng.create_material<glengine::MaterialVertexColor>(
                                                                        SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)};
    glengine::Renderable sphere_renderable = {
        eng.create_sphere_mesh(0.7),
        eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES,
                                                       SG_INDEXTYPE_UINT32)}; // phong in the original example
    // create custom renderables:
    glengine::Mesh *polyline_mesh = eng.create_mesh();
    polyline_mesh->init(create_polyline());
    // polyline_mesh->update();
    glengine::Renderable polyline_renderable = {
        polyline_mesh, eng.create_material<glengine::MaterialFlat>(SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)};
    glengine::Mesh *triangle_mesh = eng.create_mesh();
    triangle_mesh->init(triangle_vertices);
    // triangle_mesh->update();
    glengine::Renderable triangle_renderable = {triangle_mesh, eng.create_material<glengine::MaterialVertexColor>(
                                                                   SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_NONE)};

    // add renderables to the scene
    auto &grid = *eng.create_object(grid_renderable, nullptr, 101); // renderable is _copied_ in the renderobject
    auto &polyline = *eng.create_object(polyline_renderable, nullptr, 102);
    auto &triangle = *eng.create_object(triangle_renderable, nullptr, 103);
    auto &box0 = *eng.create_object(box0_renderable, nullptr, 104);
    auto &box1 = *eng.create_object(box1_renderable, nullptr, 105);
    auto &box2 = *eng.create_object(box2_renderable, nullptr, 106);
    auto &box3 = *eng.create_object(box3_renderable, nullptr, 107);
    auto &box4 = *eng.create_object(box4_renderable, nullptr, 108);
    auto &axis = *eng.create_object(axis_renderable, nullptr, 109);
    auto &sphere = *eng.create_object(sphere_renderable, nullptr, 110);

    // change object attributes
    auto *box2_mtl = (glengine::MaterialDiffuseTextured *)box2_renderable.material;
    box2_mtl->tex_diffuse = create_texture();
    box2_renderable.update_bindings();
    box2._renderables[0].update_bindings();
    // basic object hierarchy
    auto sg0 = eng.create_object({box0_renderable.mesh, eng.create_material<glengine::MaterialDiffuse>(
                                                            SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32)},
                                 nullptr, 120);
    auto sg1 = eng.create_object({box0_renderable.mesh, eng.create_material<glengine::MaterialDiffuse>(
                                                            SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32)},
                                 sg0, 121);
    auto sg2 = eng.create_object({box0_renderable.mesh, eng.create_material<glengine::MaterialDiffuse>(
                                                            SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32)},
                                 sg1, 122);
    auto sg3 = eng.create_object({box0_renderable.mesh, eng.create_material<glengine::MaterialDiffuse>(
                                                            SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32)},
                                 sg1, 123);
    sg0->set_transform(math::create_transformation<float>({1, 0, 0}, math::quat_from_euler_321<float>(0, 0, 0.5)))
        .set_scale({0.7f, 0.7f, 0.7f});
    sg1->set_transform(math::create_transformation<float>({2, 0, 0}, math::quat_from_euler_321<float>(0, 0, 0)))
        .set_scale({0.5f, 0.5f, 0.5f});
    sg2->set_transform(math::create_transformation<float>({2, 1, 0}, math::quat_from_euler_321<float>(0.3, 0, 0)));
    sg2->_renderables[0].material->color = {255, 0, 255, 255};
    sg3->set_transform(math::create_transformation<float>({2, -1, 0}, math::quat_from_euler_321<float>(0.7, 0, 0)));
    sg3->_renderables[0].material->color = {0, 255, 0, 255};
    // explicitly create and add another object to the hierarchy
    glengine::Renderable sg4_renderable = {
        eng.create_box_mesh({2, 2, 2}),
        eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32)};
    sg4_renderable.material->color = {255, 0, 0, 255};
    auto sg4 = eng.create_object(sg3, 124);
    sg4->add_renderable(&sg4_renderable, 1);
    sg4->set_transform(math::create_transformation<float>({2, -1, -3}, math::quat_from_euler_321<float>(0.7, 0, 0)));

    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f);

    eng.add_ui_function([&]() {
        ImGui::Begin("Object Info");
        auto id = 0; // eng.object_at_screen_coord(eng.cursor_pos());
        ImGui::Text("Object id: %d", id);
        ImGui::End();
    });

    (void)grid; // unused var

    int cnt = 0;
    while (eng.render()) {
        float t = glfwGetTime();
        uint8_t k1 = uint8_t((std::sin(cnt / 100.0f) + 1) / 2 * 255);
        uint8_t k2 = uint8_t((std::cos(cnt / 100.0f) + 1) / 2 * 255);

        sg4_renderable.material->color = {k1, 0, k2, 255};

        // polyline
        polyline_renderable.material->color = {k2, 0, k1, 255};
        // triangle
        triangle
            .set_transform(
                math::create_transformation({0.0f, 0.0f, 0.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)))
            .set_scale({1.0f + 0.5f * std::sin(2 * t), 1.0f, 1.0f});
        // box (static)
        box0.set_transform(
            math::create_transformation({-1.0f, 0.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)));
        box0_renderable.material->color = {k1, k2, 0, 255};
        // another box using the same mesh
        box1.set_transform(
            math::create_transformation({1.0f, 0.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)));
        box1_renderable.material->color = {k1, 0, k2, 255};
        // third box
        box2.set_transform(
                math::create_transformation({2.0f, 0.0f, 0.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)))
            .set_scale({1.5f, 1.5f, 1.5f});
        // fourth box
        box3.set_transform(
                math::create_transformation({-2.0f, -1.5f, 0.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)))
            .set_scale({0.5f, 0.5f, 0.5f});
        // modify box4 (update and draw)
        auto &bm = *box4._renderables[0].mesh;
        bm.vertices[10] = {{-0.5f + 0.2f * std::cos(3 * t), -0.5f + 0.2f * std::sin(3 * t), 0.5f}, {50, 50, 200, 255}};
        bm.vertices[15] = {{-0.5f + 0.2f * std::cos(3 * t), -0.5f + 0.2f * std::sin(3 * t), 0.5f}, {50, 50, 200, 255}};
        bm.vertices[17] = {{-0.5f + 0.2f * std::cos(3 * t), -0.5f + 0.2f * std::sin(3 * t), 0.5f}, {50, 50, 200, 255}};
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
        box4._renderables[0].update_bindings();
        box4.set_transform(
            math::create_transformation({0.0f, 0.0f, 3.0f}, math::quat_from_euler_321(0.0f, 0.0f, 0.0f)));
        // axis
        axis.set_transform(
            math::create_transformation({0.0f, -1.0f, 2.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 2.1f)));
        // sphere
        sphere.set_transform(
            math::create_transformation({0.0f, 3.0f, 1.0f}, math::quat_from_euler_321(0.0f, 0.0f, 0.0f)));
        sphere_renderable.material->color = {0, k1, k2, 255};

        // scenegraph
        sg0->set_transform(
            math::create_transformation({2.0f, 0.0f, -1.0f}, math::quat_from_euler_321(1.0f, 0.0f, t * 1.5f)));
        sg1->set_transform(math::create_transformation({2.0f + std::sin(t), 0.0f, 0.0f},
                                                       math::quat_from_euler_321(3 * t, 0.0f, 0.0f)))
            .set_visible((int(t * 2) % 2 == 1));

        cnt++;
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
