#include "math/vmath.h"

#include "gl_engine.h"
#include "gl_material_diffuse.h"
#include "gl_utils.h"
#include "imgui/imgui.h"
#include "sokol_time.h"

#include <numeric>
#include <array>

namespace glengine {
std::vector<Renderable> create_from_gltf(GLEngine &eng, const char *filename);
}

template <typename T> T rand_range(T v1, T v2) {
    return v1 + T(double(rand()) / RAND_MAX * (v2 - v1));
}

int main(int argc, char *argv[]) {

    srand(12345678);
    std::string gltf_filename = "../resources/models/DamagedHelmet/DamagedHelmet.gltf";
    if (argc > 1) {
        gltf_filename = argv[1];
    }

    glengine::GLEngine eng;
    // standard resolution and disable vsync
    glengine::Config config{.window_width = 1280, .window_height = 720, .vsync = false, .use_mrt = true};
    eng.init(config);

    eng._camera_manipulator.set_azimuth(-0.7f).set_elevation(1.3f).set_distance(4.0f);

    // /////// //
    // objects //
    // /////// //
    // create a lot of boxes
    float l = 0.5f;
    auto box_mesh = eng.create_box_mesh({l, l, l});
    // render objects
    const int N = 50;
    for (float px = 0; px <= N * l; px += l) {
        for (float py = 0; py <= N * l; py += l) {
            // tf matrices
            float x = -5.0f + px;
            float y = -5.0f + py;
            float z = -0.6f + rand_range(-0.2f, 0.1f);
            math::Matrix4f tf = math::create_transformation({x, y, z}, math::quat_from_euler_321(0.0f, 0.0f, 0.0f));
            auto *mtl = eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
            auto obj = eng.create_object({box_mesh, mtl});
            obj->set_transform(tf);
            mtl->color = {rand_range<uint8_t>(100, 255), rand_range<uint8_t>(100, 255), rand_range<uint8_t>(100, 255),
                          255};
        }
    }
    // load a gltf file if passed in the command line
    glengine::Object *gltf_obj = nullptr;
    double avg_time = 0;
    if (gltf_filename != "") {
        gltf_obj = eng.create_object();
        auto gltf_renderables = glengine::create_from_gltf(eng, gltf_filename.c_str());
        printf("loaded %d renderables from gltf file\n", (int)gltf_renderables.size());
        gltf_obj->add_renderable(gltf_renderables.data(), gltf_renderables.size());

        eng.add_ui_function([&]() {
            ImGui::Begin("Scene Info");
            ImGui::Text("frame time (ms): %f", avg_time);
            ImGui::Text("fps: %d", int(1000 / avg_time));
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
    // ///////// //
    // main loop //
    // ///////// //
    int cnt = 0;
    uint64_t curr_time = 0;
    std::array<double, 100> time_samples;
    while (eng.render()) {
        if (gltf_obj) {
            gltf_obj->set_transform(
                math::create_transformation<float>({0, 0, 0.5f}, math::quat_from_euler_321<float>(0, 0, cnt / 50.0f)));
        }

        time_samples[cnt % time_samples.size()] = stm_ms(stm_laptime(&curr_time));
        cnt++;
        avg_time = std::accumulate(time_samples.begin(), time_samples.end(), 0.0) / time_samples.size();
    }

    eng.terminate();
    return 0;
}

