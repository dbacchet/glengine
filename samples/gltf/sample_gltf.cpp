#include "math/vmath.h"

#include "gl_types.h"
#include "gl_prefabs.h"
#include "gl_engine.h"

#include <cstdlib>
#include <cstdio>

glengine::MeshData load_mesh(const char *filename) {
    return glengine::create_from_gltf(filename);
}

int main(int argc, char *argv[]) {

    glengine::GLEngine eng;
    eng.init({1280, 720, true});

    // meshes
    glengine::Mesh *grid_mesh = eng.create_grid_mesh(204, 50.0f, 1.0f);
    glengine::Mesh *axis_mesh = eng.create_axis_mesh(205);
    glengine::Mesh *model_mesh = eng.create_mesh(206);
    
    glengine::MeshData md = glengine::create_from_gltf(argv[1]);
    model_mesh->init(md.vertices, md.indices, GL_TRIANGLES);

    // render objects
    auto &grid = *eng.create_renderobject(101, grid_mesh, eng.get_stock_shader(glengine::StockShader::VertexColor));
    auto &axis = *eng.create_renderobject(110, axis_mesh, eng.get_stock_shader(glengine::StockShader::VertexColor));
    auto &model = *eng.create_renderobject(201, model_mesh, eng.get_stock_shader(glengine::StockShader::Diffuse));
    model.set_scale({0.2,0.2,0.2});

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
