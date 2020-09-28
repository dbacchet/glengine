#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_engine.h"
#include "stb/stb_image.h"

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

    eng._camera_manipulator.set_azimuth(-1.0f).set_elevation(1.0f);

    (void)grid; // unused var

    // create sensors
    LidarSensor *lidar1 = new LidarSensor("lidar1", eng);
    LidarSensor *lidar2 = new LidarSensor("lidar2", eng);
    LidarSensor *lidar3 = new LidarSensor("lidar3", eng);
    CameraSensor *camera1 = new CameraSensor("camera1", eng);
    CameraSensor *camera2 = new CameraSensor("camera2", eng);
    CameraSensor *camera3 = new CameraSensor("camera3", eng);
    // vector of sensors to simplify common calls
    std::vector<Sensor*> sensors= {lidar1, lidar2, lidar3, camera1, camera2, camera3};

    // add a UI panel to the visualizer
    eng.add_ui_function([&]() {
        ImGui::Begin("Sensors");
        for (auto &s : sensors) {
            s->draw_imgui();
        }
        ImGui::End();
        // ImGui::ShowDemoWindow();
    });

    // load a bunch of images
    constexpr uint16_t num_images = 300;
    uint8_t *images[num_images];
    int width, height, nr_channels;
    for (uint16_t i=0; i<num_images; i++) {
        char fname[128];
        sprintf(fname, "images/frame_%03d_delay-0.05s.png",i);
        stbi_set_flip_vertically_on_load(true);
        images[i] = stbi_load(fname, &width, &height, &nr_channels, 0);
    }

    // set sensor initial transform/data
    lidar1->set_transform({0.0f,0.0f,1.7f},{0.0f, 0.0f, 0.0f});
    lidar2->set_transform({0.0f,-0.7f,1.6f},{0.2f, 0.0f, 0.0f});
    lidar3->set_transform({0.0f,0.7f,1.6f},{-0.2f, 0.0f, 0.0f});
    camera1->set_transform({1.0f,0.7f,1.5f},{0.0f, 0.0f, 0.4f});
    camera2->set_transform({1.0f,-0.7f,1.5f},{0.0f, 0.0f, -0.4f});
    camera3->set_transform({-1.0f,0.0f,1.5f},{0.0f, 0.0f, M_PI});

    constexpr uint32_t num_points = 1000;
    std::vector<glengine::Vertex> points(num_points);

    int cnt = 0;
    while (eng.render()) {
        float t = glfwGetTime();
        // sensors
        for (uint32_t i = 0; i < num_points; i++) {
            float d = i / 90.0f;
            float alpha = i / 5.5f;
            points[i] = {{d * std::cos(alpha), d * std::sin(alpha), std::cos(d+t)/(d/15+5)},
                         {uint8_t(i * 11 % 100 + 100), uint8_t(i * 31 % 200 + 50), uint8_t(i * 51 % 200 + 50), 255}};
        }
        lidar1->set_points(points);
        // lidar1.set_transform( math::create_transformation({-1.0f, 0.0f, std::sin(t)}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)));
        lidar2->set_points(points);
        lidar3->set_points(points);
        int img_idx = cnt/2;
        camera1->set_image(width,height,images[img_idx%num_images],nr_channels);
        camera2->set_image(width,height,images[(img_idx+10)%num_images],nr_channels);
        camera3->set_image(width,height,images[(img_idx+20)%num_images],nr_channels);
        for (auto &s : sensors) {
            s->update();
        }

        cnt++;
    }

    eng.terminate();

    // free images
    for (uint16_t i=0; i<num_images; i++) {
        stbi_image_free(images[i]);
    }

    exit(EXIT_SUCCESS);
}
