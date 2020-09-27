#pragma once

#include "sensor.h"
#include "math/vmath.h"

#include <string>

class CameraSensor: public Sensor {
  public:
    CameraSensor(const std::string &name_, glengine::GLEngine &eng)
    : Sensor(name_, eng) {
        auto &rm = eng.resource_manager();
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

        quad = eng.create_renderobject(quad_renderable, root);
        frustum = eng.create_renderobject(frustum_renderable, root);
    }

    void update() override {
        Sensor::update();
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

    void draw_imgui() override {
        if (ImGui::CollapsingHeader(name.c_str())) {
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
            int img_width = 200;
            int img_height = 150;
            ImGui::Image((void*)(intptr_t)image->texture_id, ImVec2(img_width,img_height),ImVec2(0,1),ImVec2(1,0));
        }
    }

    float h_fov_deg = 90.0f; ///< horizontal field of view
    float v_fov_deg = 50.0f; ///< vertical field of view
    float d = 10.0f;         ///< distance of the image plane (for viz)
    glengine::Object *quad = nullptr;
    glengine::Object *frustum = nullptr;
    glengine::Mesh *quad_mesh = nullptr;
    glengine::Mesh *frustum_mesh = nullptr;
    glengine::Texture *image = nullptr;
};
