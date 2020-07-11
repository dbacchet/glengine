#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_engine.h"

#include <cstdio>
#include <cmath>
#include <vector>

struct Obj {
    glengine::ID id = 0;
    float alpha = 0.0f;
    float beta = 0.0f;
    math::Matrix4f tf;
    float vlen = 0.0f; // length of the random radial speed
    glengine::Object *ro;
};

template <typename T> T rand_range(T v1, T v2) {
    return v1 + T(double(rand()) / RAND_MAX * (v2 - v1));
}

int main(void) {
    srand(12345678);

    glengine::Config config;
    config.window_width = 1280;
    config.window_height = 720;
    config.vsync = true;
    config.show_framebuffer_texture = true;
    config.show_imgui_statistics = true;

    glengine::GLEngine eng;
    eng.init(config);
    auto &rm = eng.resource_manager();

    uint32_t N = 40;
    uint32_t M = 400;
    float R = 50.0;
    float r = float(R * N) / M;
    float l = 2 * M_PI * r / N;

    // float r = 5.0f;
    // float R = 10*r;
    // uint32_t N = 40;
    // uint32_t M = (r+R)/r*N;
    // float l = 2*M_PI*r/N;
    printf("r:%f R:%f, N:%d, M:%d, l:%f\n", r, R, N, M, l);

    std::vector<Obj> cubes(M * N);

    // meshes
    glengine::Mesh *box_mesh = rm.create_box_mesh("box_mesh", {l, l, l});
    // render objects
    for (uint32_t i = 0; i < M; i++) {
        float alpha = 2 * M_PI * i / M;
        for (uint32_t j = 0; j < N; j++) {
            float beta = 2 * M_PI * j / N;
            // tf matrices
            math::Matrix4f t1 = math::create_transformation({0.0f, 0.0f, 0.0f}, math::quat_from_euler_321(0.0f, alpha, 0.0f));
            math::Matrix4f t2 = math::create_translation<float>({0.0f, 0.0f, R});
            math::Matrix4f t3 = math::create_transformation({0.0f, 0.0f, 0.0f}, math::quat_from_euler_321(beta, 0.0f, 0.0f));
            math::Matrix4f t4 = math::create_translation<float>({0.0f, 0.0f, r - l / 2});
            auto &obj = cubes[i * N + j];
            obj.id = i * N + j;
            obj.alpha = alpha;
            obj.beta = beta;
            obj.vlen = rand_range(0.0f, 1.0f);
            obj.tf = t1 * t2 * t3 * t4;
            obj.ro = eng.create_renderobject({box_mesh, rm.create_material("",glengine::StockShader::Diffuse)}, nullptr, i * N + j);
            obj.ro->set_transform(obj.tf);
            obj.ro->_renderables[0].material->color = {rand_range<uint8_t>(80, 250), rand_range<uint8_t>(80, 250), rand_range<uint8_t>(80, 250), 255};
        }
    }

    auto &grid = *eng.create_renderobject({rm.create_grid_mesh("grid", 50.0f, 1.0f),
                                           rm.create_material("grid_mtl", glengine::StockShader::VertexColor)});

    eng._camera_manipulator.set_center({-18.33f, 1.01f, 0.0f})
        .set_azimuth(1.87f)
        .set_elevation(1.39f)
        .set_distance(26.0f);

    // ui
    eng.add_ui_function([&]() {
        ImGui::SetNextWindowSize(math::Vector2f(0, 0)); // (0,0) will adjust the size to the content
        ImGui::Begin("Camera Manipulator");
        ImGui::DragFloat3("center", eng._camera_manipulator.center());
        ImGui::SliderAngle("azimuth", &eng._camera_manipulator.azimuth());
        ImGui::SliderAngle("elevation", &eng._camera_manipulator.elevation());
        ImGui::DragFloat("distance", &eng._camera_manipulator.distance(), 1.0f, 0.01f, 100.0f);
        ImGui::End();
    });

    // main loop
    while (eng.render()) {
        float t = glfwGetTime();

        // update positions
        float da = t / 10.0f;
        math::Matrix4f m =
            math::create_transformation({0.0f, 0.0f, -R}, math::quat_from_euler_321<float>(0.0f, da, 0.0f));
        for (auto &obj : cubes) {
            float angle = obj.alpha + da - M_PI / 2;
            float scaling = float(1.0f + std::sin(angle));
            angle = obj.alpha + da;
            scaling = std::pow(std::sin(angle), 2);
            math::Matrix4f vm = math::create_translation<float>({0.0f, 0.0f, obj.vlen * scaling * 100});
            obj.ro->set_transform(m * obj.tf * vm);
            obj.ro->set_scale({1.0f + scaling, 1.0f + scaling, 1.0f + scaling});
            obj.ro->set_visible(std::cos(angle) > 0.5);
        }
    }

    eng.terminate();

    exit(EXIT_SUCCESS);
}
