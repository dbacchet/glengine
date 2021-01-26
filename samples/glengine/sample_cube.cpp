#include "math/vmath.h"

#include "gl_engine.h"
#include "gl_context_glfw.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"
#include "gl_material_diffuse.h"
#include "gl_material_flat.h"
#include "gl_material_vertexcolor.h"
#include "gl_renderable.h"

#include "uv_grid_256.png.h"


int main() {

    glengine::ContextGLFW context;
    context.init({.window_width = 1280, .window_height = 720});
    glengine::GLEngine eng;
    eng.init(&context, {});

    eng._camera_manipulator.set_azimuth(0.5f).set_elevation(0.8f);

    // /////// //
    // objects //
    // /////// //
    // grid
    // mesh
    auto *grid_mesh = eng.create_grid_mesh(50.0f, 1.0f);
    // material
    auto *grid_mtl = eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES);
    // renderable
    glengine::Renderable grid_renderable{grid_mesh, grid_mtl};
    // object
    auto *grid = eng.create_object(grid_renderable);
    // the 4 commands above can be collapsed into:
    // auto *grid = eng.create_object({eng.create_grid_mesh(50.0f, 1.0f),
    //                                 eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES)});

    // boxes
    // mesh
    auto box_md = glengine::create_box_data();
    glengine::Mesh *box_mesh = eng.create_mesh(box_md.vertices, box_md.indices);
    // material
    auto *box_mtl_vc =
        eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
    auto *box_mtl_flat = eng.create_material<glengine::MaterialFlat>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
    auto *box_mtl_flat_textured = eng.create_material<glengine::MaterialFlatTextured>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
    box_mtl_flat_textured->tex_diffuse = eng.resource_manager().get_or_create_image(uv_grid_256_png, uv_grid_256_png_len);
    auto *box_mtl_diffuse =
        eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
    auto *box_mtl_diffuse_textured =
        eng.create_material<glengine::MaterialDiffuseTextured>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
    box_mtl_diffuse_textured->tex_diffuse = box_mtl_flat_textured->tex_diffuse;
    // renderables
    glengine::Renderable box_renderable_vc{box_mesh, box_mtl_vc};
    glengine::Renderable box_renderable_flat{box_mesh, box_mtl_flat};
    glengine::Renderable box_renderable_flat_textured{box_mesh, box_mtl_flat_textured};
    glengine::Renderable box_renderable_diffuse{box_mesh, box_mtl_diffuse};
    glengine::Renderable box_renderable_diffuse_textured{box_mesh, box_mtl_diffuse_textured};
    // objects
    auto *root = eng.create_object();
    auto *box1 = eng.create_object(box_renderable_vc, root);
    auto *box2 = eng.create_object(box_renderable_flat, root);
    auto *box3 = eng.create_object(box_renderable_flat_textured, root);
    auto *box4 = eng.create_object(box_renderable_diffuse, root);
    auto *box5 = eng.create_object(box_renderable_diffuse_textured, box3);
    box5->set_transform(math::create_translation<float>({0.0f,0.0f,2.0f}));

    glengine::Renderable axes {eng.create_axis_mesh(), eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)};
    box3->add_renderable(&axes,1);

    // ///////// //
    // main loop //
    // ///////// //
    int cnt = 0;
    while (eng.render()) {
        // update material color
        box_mtl_flat->color.r = uint8_t(cnt % 256);
        box_mtl_flat->color.g = uint8_t(255 - cnt % 256);

        // update object transforms
        box1->set_transform(math::create_translation<float>({-1.0f, -1.0f, 3.0f * std::sin(cnt / 70.0f)}));
        box2->set_transform(math::create_translation<float>({-1.0f, 1.0f, 2.0f * std::sin(cnt / 60.0f)}));
        box3->set_transform(math::create_translation<float>({1.0f, 1.0f, 1.0f * std::sin(cnt / 50.0f)}));
        box4->set_transform(math::create_translation<float>({1.0f, -1.0f, 2.0f * std::sin(cnt / 40.0f)}));
        // root->set_transform(
        //     math::create_transformation({0.0f, 0.0f, 0.0f}, math::quat_from_euler_321(0.0f, 0.0f, cnt / 100.0f)));

        cnt++;
    }

    eng.terminate();
    return 0;
}

