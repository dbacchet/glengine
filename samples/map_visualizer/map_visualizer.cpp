#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_engine.h"
#include "stb/stb_image.h"

#include "pcd_tiles.h"
#include "pcd_loader.h"

#include "annotations.h"

#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

int main(void) {

    glengine::Config config;
    config.window_width = 1280;
    config.window_height = 720;
    config.vsync = true;

    glengine::GLEngine eng;
    eng.init(config);
    auto &rm = eng.resource_manager();

    // create basic renderables
    glengine::Renderable grid_renderable = {rm.create_grid_mesh("grid", 50.0f, 1.0f),
                                            rm.create_material("grid_mtl", glengine::StockShader::VertexColor)};
    // add renderables to the scene
    auto &grid = *eng.create_renderobject(grid_renderable, nullptr); // renderable is _copied_ in the renderobject

    eng._camera_manipulator.set_azimuth(-1.0f).set_elevation(1.0f);

    (void)grid; // unused var

    PCDTiles *point_cloud = new PCDTiles("point_cloud", eng);
    Annotations *annotations = new Annotations("annotations", eng);

    std::vector<Sensor *> sensors = {point_cloud, annotations};

    math::Vector3f cam_center = eng._camera_manipulator.center();
    eng._camera_manipulator.set_center({0, 0, -80});
    int32_t pcd_tile_radius = 3;

    // add a UI panel to the visualizer
    eng.add_ui_function([&]() {
        ImGui::Begin("camera");
        cam_center = eng._camera_manipulator.center();
        ImGui::DragFloat3("camera center", &cam_center[0], 0.1f, -200.0f, 200.0f);
        eng._camera_manipulator.set_center(cam_center);
        grid.set_transform(math::create_translation(cam_center));
        ImGui::Text("Grid");
        ImGui::Checkbox("grid visible", &grid._visible);
        ImGui::Text("Tiles");
        ImGui::SliderInt("radius", &pcd_tile_radius, 0, 10);
        ImGui::End();
        ImGui::Begin("Sensors");
        for (auto &s : sensors) {
            s->draw_imgui();
        }
        ImGui::End();
        // ImGui::ShowDemoWindow();
    });

    // preload some tiles
    point_cloud->update_tiles(math::Vector2i(int(cam_center.x / 20.0), int(cam_center.y / 20.0)), pcd_tile_radius);

    // load annotations
    annotations->init_from_file("/maps/Voyage_VGCC_Protobuf_v1.6_Preview_20201218/vyg_map.bin");

    int cnt = 0;
    while (eng.render()) {
        float t = glfwGetTime();
        for (auto &s : sensors) {
            s->update();
        }

        point_cloud->update_tiles(math::Vector2i(int(cam_center.x / 20.0), int(cam_center.y / 20.0)), pcd_tile_radius);

        cnt++;
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
