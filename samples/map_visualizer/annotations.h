#pragma once

#include "sensor.h"
#include "math/vmath.h"

#include "voyage_map.pb.h"
#include "wgs84_converter.hpp"

#include <string>
#include <unordered_map>
#include <fstream>


// simple class to manage the info in the map_annotations
class Annotations : public Sensor {
  public:
    Annotations(const std::string &name_, glengine::GLEngine &eng)
    : Sensor(name_, eng)
    , _eng(eng) {
        lanes_material = _eng.resource_manager().create_material("lanes_mtl", glengine::StockShader::Flat);
        lanes_material->base_color_factor = {1.0, 1.0, 1.0, 1.0};
        stopline_material = _eng.resource_manager().create_material("stopline_mtl", glengine::StockShader::Flat);
        stopline_material->base_color_factor = {1.0f, 0.0f, 0.0f, 1.0f};
        crosswalk_material = _eng.resource_manager().create_material("crosswalk_mtl", glengine::StockShader::Flat);
        crosswalk_material->base_color_factor = {1.0f, 1.0f, 0.0f, 1.0f};
    }

    void draw_imgui() override {
        if (ImGui::CollapsingHeader(name.c_str())) {
            Sensor::draw_imgui();
            char decorated_label[128];
            sprintf(decorated_label, "pos##%s", name.c_str());
            ImGui::DragFloat3(decorated_label, &pos[0], 0.1f, -200.0f, 200.0f);
            sprintf(decorated_label, "ori##%s", name.c_str());
            ImGui::DragFloat3(decorated_label, &ori_euler[0], 0.1f, -10.0f, 10.0f);
            sprintf(decorated_label, "color##%s", name.c_str());
            ImGui::ColorEdit4(decorated_label, &lanes_material->base_color_factor[0]);
        }
    }

    bool init_from_file(const char *filename,
                        const math::Vector3d &origin = {37.290493011474609375, -121.753868103027343750,
                                                        204.159072875976562500});

    Map _annotations; ///< protobuf map

    glengine::Material *lanes_material = nullptr;
    glengine::Material *stopline_material = nullptr;
    glengine::Material *crosswalk_material = nullptr;
    glengine::GLEngine &_eng;
};
