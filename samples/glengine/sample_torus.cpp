#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_engine.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"
#include "gl_material_diffuse.h"
#include "gl_material_flat.h"
#include "gl_material_vertexcolor.h"
#include "gl_renderable.h"

#include "microprofile/microprofile.h"

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

int main() {
    srand(12345678);

    glengine::GLEngine eng;
    eng.init({1280, 720, true});

    eng._camera_manipulator.set_azimuth(0.5f).set_elevation(0.8f);

    uint32_t N = 40;
    uint32_t M = 400;
    float R = 50.0;
    float r = float(R * N) / M;
    float l = 2 * M_PI * r / N;
    printf("r:%f R:%f, N:%d, M:%d, l:%f\n", r, R, N, M, l);

    std::vector<Obj> cubes(M * N);

    // meshes
    glengine::Mesh box_mesh;
    auto box_md = glengine::create_box_data({l,l,l});
    box_mesh.init(box_md.vertices, box_md.indices);
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

            auto *mtl = eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
            glengine::Renderable box_renderable {&box_mesh, mtl};
            obj.ro = eng.create_object(box_renderable, nullptr, i * N + j);
            obj.ro->set_transform(obj.tf);
            mtl->color = {rand_range<uint8_t>(60, 255), rand_range<uint8_t>(60, 255), rand_range<uint8_t>(60, 255), 255};
        }
    }

    // /////// //
    // objects //
    // /////// //
    // grid
    // mesh
    glengine::Mesh grid_mesh;
    auto grid_md = glengine::create_grid_data(50.0f);
    grid_mesh.init(grid_md.vertices);
    // material
    auto *grid_mtl = eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES);
    // renderable
    glengine::Renderable grid_renderable {&grid_mesh, grid_mtl};
    // object
    auto *grid = eng.create_object(grid_renderable);

    eng._camera_manipulator.set_center({-18.33f, 1.01f, 0.0f})
        .set_azimuth(1.87f)
        .set_elevation(1.39f)
        .set_distance(26.0f);

    // ///////// //
    // main loop //
    // ///////// //
    int cnt = 0;
    while (eng.render()) {
        float t = glfwGetTime();

        // update positions
        MICROPROFILE_ENTERI("sample_torus", "update tfs", MP_AUTO);
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
        MICROPROFILE_LEAVE();

        cnt++;
    }

    eng.terminate();
    return 0;
}

