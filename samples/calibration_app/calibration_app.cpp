#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_engine.h"

#include "sensor.h"
#include "camera_sensor.h"
#include "lidar_sensor.h"

#include <cstdio>
#include <string>
#include <vector>


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
    auto &grid = *eng.create_renderobject(grid_renderable, nullptr, 101); // renderable is _copied_ in the renderobject

    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f);

    (void)grid; // unused var

    // create sensors
    LidarSensor *lidar1 = new LidarSensor("lidar1", eng);
    LidarSensor *lidar2 = new LidarSensor("lidar2", eng);
    LidarSensor *lidar3 = new LidarSensor("lidar3", eng);
    CameraSensor *camera1 = new CameraSensor("camera1", eng);
    CameraSensor *camera2 = new CameraSensor("camera2", eng);
    // vector of sensors to simplify common calls
    std::vector<Sensor*> sensors= {lidar1, lidar2, lidar3, camera1, camera2};

    // add a UI panel to the visualizer
    eng.add_ui_function([&]() {
        ImGui::Begin("Sensors");
        for (auto &s : sensors) {
            s->draw_imgui();
        }
        ImGui::End();
        // ImGui::ShowDemoWindow();
    });

    constexpr uint32_t num_points = 1000;
    std::vector<glengine::Vertex> points(num_points);

    int cnt = 0;
    while (eng.render()) {
        float t = glfwGetTime();
        // sensors
        for (uint32_t i = 0; i < num_points; i++) {
            float d = i / 90.0f;
            float alpha = i / 5.5f;
            points[i] = {{d * std::cos(alpha), d * std::sin(alpha), std::cos(d)},
                         {uint8_t(i * 11 % 100 + 100), uint8_t(i * 31 % 200 + 50), uint8_t(i * 51 % 200 + 50), 255}};
        }
        lidar1->set_points(points);
        // lidar1.set_transform( math::create_transformation({-1.0f, 0.0f, std::sin(t)}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)));
        lidar2->set_points(points);
        lidar3->set_points(points);
        for (auto &s : sensors) {
            s->update();
        }

        cnt++;
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
