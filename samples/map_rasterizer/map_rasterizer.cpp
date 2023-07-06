#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_engine.h"
#include "gl_context_glfw.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"
#include "gl_material_diffuse.h"
#include "gl_material_flat.h"
#include "gl_material_vertexcolor.h"

#include "imgui/imgui.h"

#include "sokol_time.h"
#include "cmdline.h"

#include "json.hpp"
#include <fstream>

std::vector<glengine::Vertex> triangle_vertices = {{{-0.6f, -0.4f, 0.0f}, {255, 0, 0, 255}},
                                                   {{0.6f, -0.4f, 0.0f}, {0, 255, 0, 255}},
                                                   {{0.0f, 0.6f, 0.0f}, {0, 0, 255, 255}}};

std::vector<glengine::Vertex> create_polyline() {
    std::vector<glengine::Vertex> vertices;
    float r = 3.0f;
    for (float s = 0; s < 15.0f; s += 0.1) {
        vertices.push_back({{r * std::cos(s), r * std::sin(s), s / 3.0f}});
    }
    return vertices;
}


void create_line_segment(glengine::GLEngine &eng, glengine::Object *parent, const nlohmann::json &ls) {

    if (ls.count("point")<1) {
        return;
    }
    std::vector<glengine::Vertex> vertices;
    for (const auto &p : ls["point"]) {
        vertices.push_back({{p["x"], p["y"], p["z"]}});
    }
    if (vertices.size()<2) {
    return;
    }
    glengine::Mesh *mesh = eng.create_mesh();
    mesh->init(vertices);
    glengine::Renderable renderable = {
        mesh, eng.create_material<glengine::MaterialFlat>(SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)};
    eng.create_object(renderable, parent, 102);
}
        
void create_map_polylines(glengine::GLEngine &eng, glengine::Object *parent, const std::string &map_filename) {
    std::ifstream f(map_filename.c_str());
    auto data = nlohmann::json::parse(f);
    if (data.count("lane")>0) {
        printf("num lanes: %lu\n",data["lane"].size());
    }
    // draw all lanes, one renderable object per line segment
    for (const auto &l : data["lane"]) {
        if (l.count("leftBoundary")>0) {
            const auto &lb = l["leftBoundary"];
            if (lb.count("curve")>0 && lb["curve"].count("segment")>0) {
                for (const auto &segm : lb["curve"]["segment"]) {
                    create_line_segment(eng, parent, segm["lineSegment"]);
                }
            }
        }
        if (l.count("rightBoundary")>0) {
            const auto &rb = l["rightBoundary"];
            if (rb.count("curve")>0 && rb["curve"].count("segment")>0) {
                for (const auto &segm : rb["curve"]["segment"]) {
                    create_line_segment(eng, parent, segm["lineSegment"]);
                }
            }
        }
    }

    f.close();
}

std::vector<glengine::Vertex> create_map_polyline(const std::string &map_filename) {
    std::vector<glengine::Vertex> vertices;

    std::ifstream f(map_filename.c_str());
    auto data = nlohmann::json::parse(f);
    printf("num lanes: %lu\n",data["lane"].size());

    for (const auto &l : data["lane"]) {
        if (l.count("leftBoundary")>0) {
            const auto &lb = l["leftBoundary"];
            for (const auto &segm : lb["curve"]["segment"]) {
                const auto &ls = segm["lineSegment"];
                for (const auto &p : ls["point"]) {
                    vertices.push_back({{p["x"], p["y"], p["z"]}});
                }
            }
        }
        if (l.count("rightBoundary")>0) {
            const auto &lb = l["rightBoundary"];
            for (const auto &segm : lb["curve"]["segment"]) {
                const auto &ls = segm["lineSegment"];
                for (const auto &p : ls["point"]) {
                    vertices.push_back({{float(p["x"])/10, float(p["y"])/10, p["z"]}});
                }
            }
        }
    }

    f.close();
    // float r = 5.0f;
    // for (float s = 0; s < 15.0f; s += 0.1) {
    //     vertices.push_back({{r * std::cos(s), r * std::sin(s), s / 3.0f}});
    // }
    return vertices;
}

int main(int argc, char *argv[]) {
    stm_setup();

    cmdline::parser cl;
    cl.add<std::string>("file", 'f', "map file name", false, "");
    cl.add<uint32_t>("width", 'w', "window width", false, 1280, cmdline::range(16, 65535));
    cl.add<uint32_t>("height", 'h', "window height", false, 720, cmdline::range(16, 65535));
    cl.add("mrt", 'm', "use MRT and enable effects");
    cl.add("novsync", 'n', "disable vsync");
    cl.parse_check(argc, argv);

    std::string map_filename = cl.get<std::string>("file");
    uint32_t width = cl.get<uint32_t>("width");
    uint32_t height = cl.get<uint32_t>("height");
    bool vsync = !cl.exist("novsync");
    bool use_mrt = cl.exist("mrt");

    // create context and engine
    glengine::ContextGLFW context;
    context.init({.window_width = width, .window_height = height, .vsync = vsync});
    glengine::GLEngine::Config eng_config{
        // .show_framebuffer_texture = true,
        // .show_imgui_statistics = false,
        .use_mrt = use_mrt,
    };
    glengine::GLEngine eng;
    eng.init(&context, eng_config);


    auto &rm = eng.resource_manager();

    // create basic renderables
    glengine::Renderable grid_renderable = {
        eng.create_grid_mesh(50.0f, 0.1f),
        eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)};
    // create custom renderables:
    glengine::Mesh *polyline_mesh = eng.create_mesh();
    polyline_mesh->init(create_polyline());
    // polyline_mesh->update();
    glengine::Renderable polyline_renderable = {
        polyline_mesh, eng.create_material<glengine::MaterialFlat>(SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)};


    // add renderables to the scene
    auto &grid = *eng.create_object(grid_renderable, nullptr, 101); // renderable is _copied_ in the renderobject
    auto &polyline = *eng.create_object(polyline_renderable, nullptr, 102);

    // create map from file
    if (map_filename!="") {
        glengine::Object *map_object = eng.create_object();
        create_map_polylines(eng, map_object, map_filename);

        // glengine::Mesh *map_mesh = eng.create_mesh();
        // map_mesh->init(create_map_polyline(map_filename));
        // // map_mesh->update();
        // glengine::Renderable map_renderable = {
        //     map_mesh, eng.create_material<glengine::MaterialFlat>(SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)};
        // auto &map = *eng.create_object(map_renderable, nullptr, 102);

        eng.add_ui_function([&]() {
            ImGui::Begin("Object Info");
            auto &mat = map_object->_transform;
            ImGui::DragFloat("center x", &mat.at(3,0), 1, -1000, 1000);
            ImGui::DragFloat("center y", &mat.at(3,1), 1, -1000, 1000);
            ImGui::DragFloat("center z", &mat.at(3,2), 1, -1000, 1000);
            ImGui::End();

            ImGui::Begin("Camera Info");
            float &azimuth = eng._camera_manipulator.azimuth();
            float &elevation = eng._camera_manipulator.elevation();
            float &distance = eng._camera_manipulator.distance();
            ImGui::DragFloat("azimuth", &azimuth, 0.01, -2 * M_PI, 2 * M_PI);
            ImGui::DragFloat("elevation", &elevation, 0.01, 0, M_PI);
            ImGui::DragFloat("distance", &distance, 0.01, 0, 1000.0);
            ImGui::End();
        });
    }

    eng._camera_manipulator.set_azimuth(-0.4f).set_elevation(0.8f).set_distance(100.0f);

    eng.add_ui_function([&]() {
        ImGui::Begin("Object Info");
        auto id = 0; // eng.object_at_screen_coord(eng.cursor_pos());
        ImGui::Text("Object id: %d", id);
        ImGui::End();
    });

    (void)grid; // unused var

    int cnt = 0;
    uint64_t start_time = stm_now();
    while (eng.render()) {
        float t = float(stm_sec(stm_since(start_time)));
        uint8_t k1 = uint8_t((std::sin(cnt / 100.0f) + 1) / 2 * 255);
        uint8_t k2 = uint8_t((std::cos(cnt / 100.0f) + 1) / 2 * 255);

        // polyline
        polyline_renderable.material->color = {k2, 0, k1, 255};
        cnt++;
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
