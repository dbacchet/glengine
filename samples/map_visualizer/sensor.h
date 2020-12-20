#pragma once

#include "gl_engine.h"
#include "math/vmath.h"

#include <string>

// base class for all the sensors
class Sensor {
  public:
    Sensor(const std::string &name_, glengine::GLEngine &eng)
    : name(name_) {
        auto &rm = eng.resource_manager();
        glengine::Renderable axis_renderable = {rm.create_axis_mesh("axis"),
                                                rm.create_material("axis_mtl", glengine::StockShader::VertexColor)};
        root = eng.create_renderobject(axis_renderable);

        pos = math::Vector3f(0, 0, 0);
        ori_euler = math::Vector3f(0, 0, 0);
    }

    void set_transform(const math::Vector3f &pos_, const math::Vector3f &ori_) {
        pos = pos_;
        ori_euler = ori_;
    }

    void set_transform(const math::Matrix4f &tf) {
        // extract pos/ori from tf
        pos = math::translation(tf);
        ori_euler = math::to_euler_321(math::quat_from_matrix(tf));
    }

    void set_visible(bool visible) { root->set_visible(visible); }

    virtual void update() {
        math::Matrix4f tf =
            math::create_transformation(pos, math::quat_from_euler_321(ori_euler.x, ori_euler.y, ori_euler.z));
        root->set_transform(tf);
    }

    virtual void draw_imgui() {
        ImGui::Text("name: %s", name.c_str());
    }

    std::string name = "sensor";
    glengine::Object *root = nullptr;

    math::Vector3f pos;
    math::Vector3f ori_euler;
};
