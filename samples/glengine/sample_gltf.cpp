#include "math/vmath.h"

#include "gl_engine.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"
#include "gl_material_diffuse.h"
#include "gl_material_flat.h"
#include "gl_material_flat_textured.h"
#include "gl_material_vertexcolor.h"
#include "gl_material_pbr.h"
#include "gl_material_pbr_ibl.h"
#include "gl_renderable.h"
#include "gl_utils.h"
#include "imgui/imgui.h"

namespace glengine {
std::vector<Renderable> create_from_gltf(GLEngine &eng, const char *filename);
}

int main(int argc, char *argv[]) {

    std::string gltf_filename = "";
    if (argc > 1) {
        gltf_filename = argv[1];
    }

    glengine::GLEngine eng;
    eng.init({1280, 720, true});

    eng._camera_manipulator.set_azimuth(0.6f).set_elevation(1.2f).set_distance(5.0f);

    // /////// //
    // objects //
    // /////// //
    // grid
    auto *grid = eng.create_object({eng.create_grid_mesh(100.0f, 2.0f),
                                    eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES)});

    // load a gltf file if passed in the command line
    glengine::Object *gltf_obj = nullptr;
    bool rotate = false;
    if (gltf_filename != "") {
        gltf_obj = eng.create_object();
        auto gltf_renderables = glengine::create_from_gltf(eng, gltf_filename.c_str());
        printf("loaded %d renderables from gltf file\n", (int)gltf_renderables.size());
        gltf_obj->add_renderable(gltf_renderables.data(), gltf_renderables.size());

        // approximate camera placement using object extent
        auto aabb = glengine::calc_bounding_box(gltf_obj, true);
        printf("object bbox - center (%f,%f,%f) - size (%f,%f,%f)\n", aabb.center.x, aabb.center.y, aabb.center.z,
               aabb.size.x, aabb.size.y, aabb.size.z);
        eng._camera_manipulator.set_center(aabb.center);
        eng._camera_manipulator.set_distance(1.0f * math::length(aabb.size));

        if (argc > 2) {
            float scale = std::atof(argv[2]);
            gltf_obj->set_scale({scale, scale, scale});
        }

        // edit the first material
        auto *mat = gltf_obj->_renderables[0].material;

        eng.add_ui_function([&]() {
            ImGui::Begin("Object Info");
            auto *m = (glengine::MaterialPBRIBL *)mat;
            ImGui::DragFloat("metallic factor", &m->metallic_factor, 0.01, 0, 1);
            ImGui::DragFloat("roughness factor", &m->roughness_factor, 0.01, 0, 1);
            ImGui::Checkbox("rotate", &rotate);
            ImGui::End();
            ImGui::Begin("Camera Info");

            float &azimuth = eng._camera_manipulator.azimuth();
            float &elevation = eng._camera_manipulator.elevation();
            float &distance = eng._camera_manipulator.distance();
            ImGui::DragFloat("azimuth", &azimuth, 0.01, -2*M_PI, 2*M_PI);
            ImGui::DragFloat("elevation", &elevation, 0.01, 0, M_PI);
            ImGui::DragFloat("distance", &distance, 0.01, 0, 1000.0);
            ImGui::End();
        });
    }
    // ///////// //
    // main loop //
    // ///////// //
    int cnt = 0;
    while (eng.render()) {
        if (gltf_obj && rotate) {
            gltf_obj->set_transform(
                math::create_transformation<float>({0, 0, 0}, math::quat_from_euler_321<float>(0, 0, cnt / 50.0f)));
            cnt++;
        }
    }

    eng.terminate();
    return 0;
}

