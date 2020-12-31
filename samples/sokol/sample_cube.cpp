#include "math/vmath.h"

#include "gl_engine.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"
#include "gl_material_diffuse.h"
#include "gl_material_flat.h"
#include "gl_material_vertexcolor.h"
#include "gl_renderable.h"


int main() {

    glengine::GLEngine eng;
    eng.init({1280, 720, true});

    eng._camera_manipulator.set_azimuth(0.5f).set_elevation(0.8f);

    // /////// //
    // objects //
    // /////// //
    // grid
    // mesh
    glengine::Mesh grid_mesh(101,"grid_mesh");
    auto grid_md = glengine::create_grid_data(50.0f);
    grid_mesh.init(grid_md.vertices);
    // material
    auto *grid_mtl = eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES);
    // renderable
    glengine::Renderable grid_renderable {&grid_mesh, grid_mtl};
    grid_renderable.update_bindings();
    // object
    auto *grid = eng.create_object(grid_renderable);

    // boxes
    // mesh
    glengine::Mesh box_mesh(102,"box_mesh");
    auto box_md = glengine::create_box_data();
    box_mesh.init(box_md.vertices, box_md.indices);
    // material
    auto *box_mtl_vc = eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
    auto *box_mtl_flat = eng.create_material<glengine::MaterialFlat>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
    auto *box_mtl_diffuse = eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
    // renderables
    glengine::Renderable box_renderable_vc {&box_mesh, box_mtl_vc};
    box_renderable_vc.update_bindings();
    glengine::Renderable box_renderable_flat {&box_mesh, box_mtl_flat};
    box_renderable_flat.update_bindings();
    glengine::Renderable box_renderable_diffuse {&box_mesh, box_mtl_diffuse};
    box_renderable_diffuse.update_bindings();
    // objects
    auto *root = eng.create_object();
    auto *box1 = eng.create_object(box_renderable_vc,root);
    auto *box2 = eng.create_object(box_renderable_flat,root);
    auto *box3 = eng.create_object(box_renderable_vc,root);
    auto *box4 = eng.create_object(box_renderable_diffuse,root);

    // ///////// //
    // main loop //
    // ///////// //
    int cnt = 0;
    while (eng.render()) {
        // update material color
        box_mtl_flat->color.r = uint8_t(cnt%256);
        box_mtl_flat->color.g = uint8_t(255-cnt%256);

        // update object transforms
        box1->set_transform(math::create_translation<float>({-1.0f, -1.0f, 3.0f*std::sin(cnt/70.0f)}));
        box2->set_transform(math::create_translation<float>({-1.0f, 1.0f, 2.0f*std::sin(cnt/60.0f)}));
        box3->set_transform(math::create_translation<float>({1.0f, 1.0f, 1.0f*std::sin(cnt/50.0f)}));
        box4->set_transform(math::create_translation<float>({1.0f, -1.0f, 2.0f*std::sin(cnt/40.0f)}));
        root->set_transform(math::create_transformation({0.0f,0.0f,0.0f},math::quat_from_euler_321(0.0f,0.0f,cnt/100.0f)));

        cnt++;
    }

    eng.terminate();
    return 0;
}

