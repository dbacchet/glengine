#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_engine.h"
#include "stb/stb_image.h"

#include "pcd_tiles.h"
#include "pcd_loader.h"

#include "voyage_map.pb.h"

#include "WGS84toCartesian.hpp"

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

    std::vector<Sensor*> sensors= {point_cloud};
    
    math::Vector3f cam_center = eng._camera_manipulator.center();
    eng._camera_manipulator.set_center({0,0,-80});
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
    point_cloud->update_tiles(math::Vector2i(int(cam_center.x/20.0),int(cam_center.y/20.0)),pcd_tile_radius);

    // load the map from the binary protobuf
    std::string map_file_path("/maps/Voyage_VGCC_Protobuf_v1.6_Preview_20201218/vyg_map.bin");
    std::fstream input(map_file_path.c_str(), std::ios::in | std::ios::binary);
    Map annotations;
    annotations.ParseFromIstream(&input);
    printf("number of paths: %d\n", annotations.paths_size());

    // WGS84 converter
    // "origin": {
    //   "lat": "37.290493011474609375",
    //   "lng": "-121.753868103027343750",
    //   "alt": "204.159072875976562500"
    // },
    std::array<double,2> origin {37.290493011474609375,-121.753868103027343750};
    double origin_z = 204.159072875976562500;

    std::vector<glengine::Vertex> lines_points;
    lines_points.reserve(10000);
    for (const auto &it : annotations.paths()) {
        // printf("%d\n", it.first);
        const auto &left_pline = it.second.left_boundary().line();
        for (int i=1; i<left_pline.waypoints_size(); i++) {
            const auto &pt0 = left_pline.waypoints(i-1);
            const auto &pt1 = left_pline.waypoints(i);
            std::array<double, 2> pt0_cart{wgs84::toCartesian(origin, {pt0.y(),pt0.x()})};
            std::array<double, 2> pt1_cart{wgs84::toCartesian(origin, {pt1.y(),pt1.x()})};
            lines_points.push_back({{float(pt0_cart[0]),float(pt0_cart[1]),float(pt0.z()-origin_z)}});
            lines_points.push_back({{float(pt1_cart[0]),float(pt1_cart[1]),float(pt1.z()-origin_z)}});
        }
        const auto &right_pline = it.second.right_boundary().line();
        for (int i=1; i<right_pline.waypoints_size(); i++) {
            const auto &pt0 = right_pline.waypoints(i-1);
            const auto &pt1 = right_pline.waypoints(i);
            std::array<double, 2> pt0_cart{wgs84::toCartesian(origin, {pt0.y(),pt0.x()})};
            std::array<double, 2> pt1_cart{wgs84::toCartesian(origin, {pt1.y(),pt1.x()})};
            lines_points.push_back({{float(pt0_cart[0]),float(pt0_cart[1]),float(pt0.z()-origin_z)}});
            lines_points.push_back({{float(pt1_cart[0]),float(pt1_cart[1]),float(pt1.z()-origin_z)}});
        }
        if (left_pline.waypoints_size()>0) {
            const auto &pt0 = left_pline.waypoints(0);
            const auto &pt1 = right_pline.waypoints(0);
            std::array<double, 2> pt0_cart{wgs84::toCartesian(origin, {pt0.y(),pt0.x()})};
            std::array<double, 2> pt1_cart{wgs84::toCartesian(origin, {pt1.y(),pt1.x()})};
            lines_points.push_back({{float(pt0_cart[0]),float(pt0_cart[1]),float(pt0.z()-origin_z)}});
            lines_points.push_back({{float(pt1_cart[0]),float(pt1_cart[1]),float(pt1.z()-origin_z)}});
        }
        if (left_pline.waypoints_size()>0) {
            const auto &pt0 = left_pline.waypoints(left_pline.waypoints_size()-1);
            const auto &pt1 = right_pline.waypoints(right_pline.waypoints_size()-1);
            std::array<double, 2> pt0_cart{wgs84::toCartesian(origin, {pt0.y(),pt0.x()})};
            std::array<double, 2> pt1_cart{wgs84::toCartesian(origin, {pt1.y(),pt1.x()})};
            lines_points.push_back({{float(pt0_cart[0]),float(pt0_cart[1]),float(pt0.z()-origin_z)}});
            lines_points.push_back({{float(pt1_cart[0]),float(pt1_cart[1]),float(pt1.z()-origin_z)}});
        }

        // extract tags
        auto stopline_mtl = rm.create_material("stopline_mtl", glengine::StockShader::Diffuse);
        stopline_mtl->base_color_factor = {1.0f,1.0f,0.0f,1.0f};
        for (int i=0; i<it.second.tags_size(); i++) {
            const auto &tag = it.second.tags(i);
            const auto &sp = tag.start();
            const auto &ep = tag.end();
            std::cout << tag.tag_type() << " " << sp.x() << " " << sp.y() << " " << sp.z() << " " << ep.x() << " " << ep.y() << " " << ep.z() << std::endl;
            if (tag.tag_type()==STOPLINE) {
                glengine::Renderable stopline_renderable = {rm.create_sphere_mesh(), stopline_mtl};
                auto stopline_ro = eng.create_renderobject(stopline_renderable);
                std::array<double, 2> sp_cart{wgs84::toCartesian(origin, {sp.y(),sp.x()})};
                stopline_ro->set_transform(math::create_translation<float>({float(sp_cart[0]),float(sp_cart[1]),float(sp.z()-origin_z)}));
            }
        }
    }
    glengine::Mesh *annotations_mesh = rm.create_mesh("left_lines");
    annotations_mesh->init(lines_points,GL_LINES);
    glengine::Renderable annotations_renderable = {annotations_mesh,
                                            rm.create_material("left_lines_mtl", glengine::StockShader::Flat)};
    auto &annotations_ro = *eng.create_renderobject(annotations_renderable, nullptr); // renderable is _copied_ in the renderobject

    int cnt = 0;
    while (eng.render()) {
        float t = glfwGetTime();
        for (auto &s : sensors) {
            s->update();
        }

        point_cloud->update_tiles(math::Vector2i(int(cam_center.x/20.0),int(cam_center.y/20.0)),pcd_tile_radius);

        cnt++;
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
