#pragma once

#include "sensor.h"
#include "math/vmath.h"

#include <string>

/// simple class to fetch and visualize the terrain elevation
/// The class uses the Google Maps Elevation Apis, and requires a 
class Terrain : public Sensor {
  public:
    Terrain(const std::string &name_, glengine::GLEngine &eng)
    : Sensor(name_, eng)
    , _eng(eng) {
        terrain_material = _eng.resource_manager().create_material("terrain_mtl", glengine::StockShader::Flat);
        terrain_material->base_color_factor = {0.1, 0.1, 0.1, 1.0};
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
            ImGui::ColorEdit4(decorated_label, &terrain_material->base_color_factor[0]);
        }
    }

    bool fetch_elevation_data(const math::Vector3d &origin);

    /// get the interpolated point in the grid
    math::Vector3f interpolate(double x, double y);

    std::vector<math::Vector3d> grid;
    int32_t radius = 100;   ///< how many tiles in each direction starting from origin
    double tile_len = 20.0; ///< length of each tile (m)

    glengine::Material *terrain_material = nullptr;
    glengine::GLEngine &_eng;
};
