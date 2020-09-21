#include "math/vmath.h"

#include "gl_types.h"
#include "gl_engine.h"

#include <cstdlib>
#include <cstdio>

int main(int argc, char *argv[]) {

    glengine::GLEngine eng;
    eng.init({1280, 720, true});
    auto &rm = eng.resource_manager();

    // renderables
    glengine::Renderable grid_renderable = {rm.create_grid_mesh("grid", 100.0f, 5.0f),
                                            rm.create_material("grid_mtl", glengine::StockShader::VertexColor)};
    glengine::Renderable axis_renderable = {rm.create_axis_mesh("axis"),
                                            rm.create_material("axis_mtl", glengine::StockShader::VertexColor)};
    std::vector<glengine::Renderable> model_renderables = rm.create_mesh_from_file(argv[1]);

    // render objects
    auto &grid = *eng.create_renderobject(grid_renderable);
    auto &axis = *eng.create_renderobject(axis_renderable);
    auto &model = *eng.create_renderobject(model_renderables, nullptr, 101);
    // model.set_scale({0.2,0.2,0.2});

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
        float t = glfwGetTime();
        // axis
        axis.set_transform( math::create_transformation({0.0f, -1.0f, 2.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 2.1f)));
        model.set_transform( math::create_transformation({0.0f, 0.0f, 0.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 0.5f)));
    }

    eng.terminate();

    return 0;
}
