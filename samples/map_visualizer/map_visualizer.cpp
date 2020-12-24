#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_engine.h"
#include "stb/stb_image.h"

#include "pcd_tiles.h"
#include "pcd_loader.h"

#include "annotations.h"
#include "terrain.h"

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
    grid.set_visible(false);


    (void)grid; // unused var

    PCDTiles *point_cloud = new PCDTiles("point_cloud", eng);
    Annotations *annotations = new Annotations("annotations", eng);
    Terrain *terrain = new Terrain("terrain", eng);

    std::vector<Sensor *> sensors = {point_cloud, annotations, terrain};

    eng._camera_manipulator.set_azimuth(-1.0f).set_elevation(1.0f).set_distance(150.0f);
    math::Vector3f cam_center = eng._camera_manipulator.center();
    bool follow_terrain = false;
    int32_t pcd_tile_radius = 3;

    // add a UI panel to the visualizer
    eng.add_ui_function([&]() {
        ImGui::Begin("camera");
        cam_center = eng._camera_manipulator.center();
        ImGui::DragFloat3("camera center", &cam_center[0], 0.1f, -200.0f, 200.0f);
        ImGui::Checkbox("follow terrain", &follow_terrain);
        if (terrain && follow_terrain) {
            cam_center.z = terrain->interpolate(cam_center.x, cam_center.y).z;
        }
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

    math::Vector3d origin = {37.290493011474609375, -121.753868103027343750, 204.159072875976562500};
    // preload some tiles
    point_cloud->update_tiles(math::Vector2i(int(cam_center.x / 20.0), int(cam_center.y / 20.0)), pcd_tile_radius);

    // load annotations
    annotations->init_from_file("/maps/Voyage_VGCC_Protobuf_v1.6_Preview_20201218/vyg_map.bin", origin);
        
    // load terrain
    follow_terrain = terrain->fetch_elevation_data(origin);

    while (eng.render()) {
        // update sensors
        for (auto &s : sensors) {
            s->update();
        }

        if (point_cloud->root->visible())
        point_cloud->update_tiles(math::Vector2i(int(cam_center.x / 20.0), int(cam_center.y / 20.0)), pcd_tile_radius);
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
