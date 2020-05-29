#include "math/vmath.h"

#include "gl_types.h"
#include "gl_prefabs.h"
#include "gl_engine.h"

#include <cstdlib>
#include <cstdio>

int main(int argc, char *argv[]) {

    glengine::GLEngine eng;
    eng.init({1280, 720, true});
    auto &rm = eng.resource_manager();

    // return 0;
    // meshes
    glengine::Mesh *grid_mesh = rm.create_grid_mesh(50.0f, 1.0f);
    glengine::Mesh *axis_mesh = rm.create_axis_mesh();
    std::vector<glengine::Mesh*> model_meshes = rm.create_mesh_from_file(argv[1]);
    
    // render objects
    auto &grid = *eng.create_renderobject(101, grid_mesh, rm.get_stock_shader(glengine::StockShader::VertexColor));
    auto &axis = *eng.create_renderobject(110, axis_mesh, rm.get_stock_shader(glengine::StockShader::VertexColor));
    auto &model = *eng.create_renderobject(201, model_meshes, rm.get_stock_shader(glengine::StockShader::DiffuseTextured));
    model.set_scale({0.2,0.2,0.2}).set_color({200,200,200,255});

    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f).set_distance(50.0f);

    eng.add_ui_function([&]() {
        ImGui::Begin("Object Info");
        auto id = eng.object_at_screen_coord(eng.cursor_pos());
        ImGui::Text("Object id: %d", id);
        ImGui::End();
    });

    while (eng.render()) {
        float t = glfwGetTime();
        // axis
        axis.set_transform( math::create_transformation({0.0f, -1.0f, 2.0f}, math::quat_from_euler_321(0.0f, 0.0f, t * 2.1f)));
    }

    eng.terminate();

    return 0;
}
