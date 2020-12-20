#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_engine.h"
#include "stb/stb_image.h"

#include "lidar_sensor.h"
#include "pcd_loader.h"

#include <cstdio>
#include <string>
#include <vector>
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

    LidarSensor *point_cloud = new LidarSensor("point_cloud", eng);

    std::vector<Sensor*> sensors= {point_cloud};

    // add a UI panel to the visualizer
    eng.add_ui_function([&]() {
        ImGui::Begin("Sensors");
        for (auto &s : sensors) {
            s->draw_imgui();
        }
        ImGui::End();
        // ImGui::ShowDemoWindow();
    });

    // load a sample pcl file
    std::vector<glengine::Vertex> points;
    std::string path = "/maps/cache/pcds/springfield/2.3.2/point_clouds";
    // std::string path = "/maps/cache/pcds/vgcc/2.13.2/point_clouds";
    int cnt = 0;
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
      printf("%s\n", entry.path().c_str());
      load_pcd(entry.path().c_str(), points);
      if (cnt++>1000)
        break;
    }

    point_cloud->set_points(points);


    cnt = 0;
    while (eng.render()) {
        float t = glfwGetTime();
        for (auto &s : sensors) {
            s->update();
        }

        cnt++;
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
