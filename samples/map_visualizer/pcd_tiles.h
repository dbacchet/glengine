#pragma once

#include "sensor.h"
#include "math/vmath.h"

#include <string>
#include <unordered_map>

// simple class that creates a bunch of tiles, and manages loading/unloading
class PCDTiles : public Sensor {
  public:
    PCDTiles(const std::string &name_, glengine::GLEngine &eng)
    : Sensor(name_, eng), _eng(eng) {
        points_material = _eng.resource_manager().create_material("points_mtl", glengine::StockShader::Flat);
        points_material->base_color_factor = {0.3,0.3,0.3,1.0};
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
            ImGui::ColorEdit4(decorated_label, &points_material->base_color_factor[0]);
        }
    }

    void update_tiles(const math::Vector2i &center, int32_t radius = 10);

    glengine::Material *points_material = nullptr;
    glengine::GLEngine &_eng;

    std::string tile_dir = "/maps/cache/pcds/vgcc/2.13.2/point_clouds";
    math::Vector2i tile_center = {0, 0};
    int32_t radius = -1;

    std::unordered_map<std::string, glengine::Object*> _tile_objects;
};
