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

#include "sensor.h"
#include "lidar_sensor.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>

class CameraSensor {
  public:
    CameraSensor(const std::string &name_, glengine::GLEngine &eng)
    : name(name_) {
        auto &rm = eng.resource_manager();
        // axis
        glengine::Renderable axis_renderable = {rm.create_axis_mesh("axis"),
                                                rm.create_material("axis_mtl", glengine::StockShader::VertexColor)};
        // quad
        quad_mesh = rm.create_quad_mesh("quad");
        glengine::Renderable quad_renderable = {quad_mesh,
                                                rm.create_material("quad_mtl", glengine::StockShader::DiffuseTextured)};
        image = rm.create_texture_from_file("uv_grid_256.png");
        quad_renderable.material->_textures[(uint8_t)glengine::Material::TextureType::BaseColor] = image;
        // frustum
        frustum_mesh = rm.create_mesh("frustum");
        frustum_mesh->init({{{0, 0, 0}, {30, 30, 30, 255}},
                            {{0, 0, 1}, {30, 30, 30, 255}},
                            {{0, 0, 0}, {30, 30, 30, 255}},
                            {{0, 0, 1}, {30, 30, 30, 255}},
                            {{0, 0, 0}, {30, 30, 30, 255}},
                            {{0, 0, 1}, {30, 30, 30, 255}},
                            {{0, 0, 0}, {30, 30, 30, 255}},
                            {{0, 0, 1}, {30, 30, 30, 255}}},
                           GL_LINES);
        glengine::Renderable frustum_renderable = {
            frustum_mesh, rm.create_material("frustum_mtl", glengine::StockShader::VertexColor)};

        axis = eng.create_renderobject(axis_renderable);
        quad = eng.create_renderobject(quad_renderable, axis);
        frustum = eng.create_renderobject(frustum_renderable, axis);

        pos = math::Vector3f(0, 0, 0);
        ori_euler = math::Vector3f(0, 0, 0);
    }

    void set_transform(const math::Matrix4f &tf) {
        axis->set_transform(tf);
        // extract pos/ori from tf
        pos = math::translation(tf);
        ori_euler = math::to_euler_321(math::quat_from_matrix(tf));
    }

    void update() {
        // update transformation
        math::Matrix4f tf =
            math::create_transformation(pos, math::quat_from_euler_321(ori_euler.x, ori_euler.y, ori_euler.z));
        axis->set_transform(tf);
        // place image at the right distance in the visualizer
        float lh = d * std::tan(h_fov_deg / 2 / 180.0 * M_PI);
        float lv = d * std::tan(v_fov_deg / 2 / 180.0 * M_PI);
        auto &vertices = quad_mesh->vertices;
        vertices[0].pos = {-lh, -lv, -d};
        vertices[1].pos = {lh, -lv, -d};
        vertices[2].pos = {lh, lv, -d};
        vertices[3].pos = {-lh, lv, -d};
        quad_mesh->update();
        auto &lines = frustum_mesh->vertices;
        lines[0].pos = {0, 0, 0};
        lines[1].pos = {-lh, -lv, -d};
        lines[2].pos = {0, 0, 0};
        lines[3].pos = {lh, -lv, -d};
        lines[4].pos = {0, 0, 0};
        lines[5].pos = {lh, lv, -d};
        lines[6].pos = {0, 0, 0};
        lines[7].pos = {-lh, lv, -d};
        frustum_mesh->update();
    }

    void set_visible(bool visible) { axis->set_visible(visible); }

    void draw_imgui() {
        ImGui::Text("name: %s", name.c_str());
        char decorated_label[128];
        sprintf(decorated_label, "pos##%s", name.c_str());
        ImGui::DragFloat3(decorated_label, &pos[0], 0.1f, -10.0f, 10.0f);
        sprintf(decorated_label, "ori##%s", name.c_str());
        ImGui::DragFloat3(decorated_label, &ori_euler[0], 0.1f, -10.0f, 10.0f);
        sprintf(decorated_label, "horizontal FoV##%s", name.c_str());
        ImGui::DragFloat(decorated_label, &h_fov_deg, 0.1f, 0.0f, 150.0f);
        sprintf(decorated_label, "vertical FoV##%s", name.c_str());
        ImGui::DragFloat(decorated_label, &v_fov_deg, 0.1f, 0.0f, 150.0f);
        sprintf(decorated_label, "image dist (viz)##%s", name.c_str());
        ImGui::DragFloat(decorated_label, &d, 0.1f, 0.0f, 20.0f);
    }

    std::string name = "camera";
    float h_fov_deg = 90.0f; ///< horizontal field of view
    float v_fov_deg = 50.0f; ///< vertical field of view
    float d = 10.0f;         ///< distance of the image plane (for viz)
    glengine::Object *axis = nullptr;
    glengine::Object *quad = nullptr;
    glengine::Object *frustum = nullptr;
    glengine::Mesh *quad_mesh = nullptr;
    glengine::Mesh *frustum_mesh = nullptr;
    glengine::Texture *image = nullptr;

    math::Vector3f pos;
    math::Vector3f ori_euler;
};

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
    // add renderables to the scene
    auto &grid = *eng.create_renderobject(grid_renderable, nullptr, 101); // renderable is _copied_ in the renderobject

    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f);

    (void)grid; // unused var

    // create sensors
    LidarSensor lidar1("lidar1", eng);
    LidarSensor lidar2("lidar2", eng);
    LidarSensor lidar3("lidar3", eng);
    CameraSensor camera1("camera1", eng);
    CameraSensor camera2("camera2", eng);

    eng.add_ui_function([&]() {
        ImGui::Begin("Sensors");
        lidar1.draw_imgui();
        lidar2.draw_imgui();
        lidar3.draw_imgui();
        camera1.draw_imgui();
        camera2.draw_imgui();
        ImGui::End();
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
        lidar1.set_points(points);
        // lidar1.set_transform( math::create_transformation({-1.0f, 0.0f, std::sin(t)}, math::quat_from_euler_321(0.0f, 0.0f, t * 1.5f)));
        lidar1.update();
        lidar2.set_points(points);
        lidar2.update();
        lidar3.set_points(points);
        lidar3.update();
        camera1.update();
        camera2.update();

        cnt++;
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
