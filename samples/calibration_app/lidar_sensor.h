#pragma once

#include "sensor.h"
#include "math/vmath.h"

#include <string>

class LidarSensor: public Sensor {
  public:
    LidarSensor(const std::string &name_, glengine::GLEngine &eng)
    : Sensor(name_, eng) {
        auto &rm = eng.resource_manager();
        points_mesh = rm.create_mesh("points");
        points_mesh->init({}, GL_POINTS);
        glengine::Renderable points_renderable = {points_mesh,
                                                  rm.create_material("points_mtl", glengine::StockShader::VertexColor)};
        points = eng.create_renderobject(points_renderable, root);
    }

    void set_points(const std::vector<glengine::Vertex> &pts) {
        points_mesh->vertices.assign(pts.begin(), pts.end());
        points_mesh->update();
    }

    void draw_imgui() override {
        if (ImGui::CollapsingHeader(name.c_str())) {
            ImGui::Text("name: %s", name.c_str());
            char decorated_label[128];
            sprintf(decorated_label, "pos##%s", name.c_str());
            ImGui::DragFloat3(decorated_label, &pos[0], 0.1f, -10.0f, 10.0f);
            sprintf(decorated_label, "ori##%s", name.c_str());
            ImGui::DragFloat3(decorated_label, &ori_euler[0], 0.1f, -10.0f, 10.0f);
        }
    }

    glengine::Object *points = nullptr;
    glengine::Mesh *points_mesh = nullptr;
};
