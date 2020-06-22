#include "math/vmath.h"

#include "gl_types.h"
#include "gl_engine.h"

#include <cstdlib>
#include <cstdio>

int main(int argc, char *argv[]) {

    glengine::GLEngine eng;
    glengine::Config config;
    config.window_width = 1280;
    config.window_height = 720;
    config.vsync = true;
    config.show_imgui_statistics = true;
    config.show_framebuffer_texture = true;
    eng.init(config);
    auto &rm = eng.resource_manager();

    // return 0;
    // meshes
    glengine::Mesh *grid_mesh = rm.create_grid_mesh("grid_mesh", 50.0f, 1.0f);
    glengine::Mesh *axis_mesh = rm.create_axis_mesh();
    std::vector<glengine::Renderable> model_renderables = rm.create_mesh_from_file(argv[1]);
    
    // render objects
    auto &grid = *eng.create_renderobject(grid_mesh, rm.get_stock_shader(glengine::StockShader::VertexColor));
    // auto &model = *eng.create_renderobject(model_meshes, rm.get_stock_shader(glengine::StockShader::DiffuseTextured), nullptr, 101);
    auto &model = *eng.create_renderobject(model_renderables, nullptr, 101);
    model.set_scale({0.2,0.2,0.2})
        // .set_color({200,200,200,255})
        ;

    (void)grid; // unused var

    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f).set_distance(50.0f);

    eng.add_ui_function([&]() {
        ImGui::Begin("Object Info");
        auto id = eng.object_at_screen_coord(eng.cursor_pos());
        ImGui::Text("Object id: %d", id);
        if (ImGui::Button("take screenshot")) {
            eng.save_screenshot("screenshot.png");
        }
        ImGui::End();
    });

    while (eng.render()) {
    }

    eng.terminate();

    return 0;
}
