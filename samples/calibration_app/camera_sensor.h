#pragma once

#include "sensor.h"
#include "math/vmath.h"

#include <string>

class CameraSensor : public Sensor {
  public:
    CameraSensor(const std::string &name_, glengine::GLEngine &eng)
    : Sensor(name_, eng) {
        auto &rm = eng.resource_manager();
        // quad
        std::vector<glengine::Vertex> vertices = {
            {{0.0f, 1.0f, -1.0f}, {255, 255, 255, 255}, {-1, 0, 0}, {0, 0}},
            {{0.0f, -1.0f, -1.0f}, {255, 255, 255, 255}, {-1, 0, 0}, {1, 0}},
            {{0.0f, -1.0f, 1.0f}, {255, 255, 255, 255}, {-1, 0, 0}, {1, 1}},
            {{0.0f, 1.0f, 1.0f}, {255, 255, 255, 255}, {-1, 0, 0}, {0, 1}},
        };
        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
        quad_mesh = rm.create_mesh(name.c_str());
        quad_mesh->init(vertices, indices, GL_TRIANGLES);
        glengine::Renderable quad_renderable = {quad_mesh,
                                                rm.create_material("quad_mtl", glengine::StockShader::FlatTextured)};
        uint8_t default_tex[4 * 4] = {255, 255, 255, 255};
        image = rm.create_texture_from_data(name_.c_str(), 1, 1, 4, default_tex);
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

    bool set_image(uint32_t width, uint32_t height, const uint8_t *data, int channels = 4 /*RGBA*/) {
        return image->init(width, height, data, GL_SRGB_ALPHA, channels < 4 ? GL_RGB : GL_RGBA);
    }

    void update() override {
        Sensor::update();
        // place image at the right distance in the visualizer
        float lh = d * std::tan(h_fov_deg / 2 / 180.0 * M_PI);
        float lv = d * std::tan(v_fov_deg / 2 / 180.0 * M_PI);
        auto &vertices = quad_mesh->vertices;
        vertices[0].pos = {d, lh, -lv};
        vertices[1].pos = {d, -lh, -lv};
        vertices[2].pos = {d, -lh, lv};
        vertices[3].pos = {d, lh, lv};
        quad_mesh->update();
        auto &lines = frustum_mesh->vertices;
        lines[0].pos = {0, 0, 0};
        lines[1].pos = {d, lh, -lv};
        lines[2].pos = {0, 0, 0};
        lines[3].pos = {d, -lh, -lv};
        lines[4].pos = {0, 0, 0};
        lines[5].pos = {d, -lh, lv};
        lines[6].pos = {0, 0, 0};
        lines[7].pos = {d, lh, lv};
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
            ImGui::Image((void *)(intptr_t)image->texture_id, ImVec2(img_width, img_height), ImVec2(0, 1),
                         ImVec2(1, 0));
        }
    }

    float h_fov_deg = 90.0f; ///< horizontal field of view
    float v_fov_deg = 50.0f; ///< vertical field of view
    float d = 5.0f;          ///< distance of the image plane (for viz)
    glengine::Object *quad = nullptr;
    glengine::Object *frustum = nullptr;
    glengine::Mesh *quad_mesh = nullptr;
    glengine::Mesh *frustum_mesh = nullptr;
    glengine::Texture *image = nullptr;
};
