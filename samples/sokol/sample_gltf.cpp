#include "math/vmath.h"

#include "gl_engine.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"
#include "gl_material_diffuse.h"
#include "gl_material_flat.h"
#include "gl_material_flat_textured.h"
#include "gl_material_vertexcolor.h"
#include "gl_renderable.h"

#include "uv_grid_256.png.h"
#include "stb/stb_image.h"

sg_image create_texture() {
    int img_width, img_height, num_channels;
    const int desired_channels = 4;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc *pixels = stbi_load_from_memory(uv_grid_256_png, (int)uv_grid_256_png_len, &img_width, &img_height,
                                            &num_channels, desired_channels);
    if (pixels) {
        sg_image img = sg_make_image((sg_image_desc){
            .width = img_width,
            .height = img_height,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .content.subimage[0][0] =
                {
                    .ptr = pixels,
                    .size = img_width * img_height * 4,
                },
            .label = "grid-256-png",
        });
        stbi_image_free(pixels);
        return img;
    }
    return {SG_INVALID_ID};
}


void gltf_scene_init(void);
void frame(void);

namespace glengine {
std::vector<Renderable> create_from_gltf(GLEngine &eng, const char *filename);
}

int main(int argc, char *argv[]) {

    std::string gltf_filename = "";
    if (argc>1) {
        gltf_filename = argv[1];
    }

    glengine::GLEngine eng;
    eng.init({1280, 720, true});

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

    // load a gltf file if passed in the command line
    if (gltf_filename != "") {
        auto *gltf_obj = eng.create_object();
        auto gltf_renderables = glengine::create_from_gltf(eng, gltf_filename.c_str());
        printf("loaded %d renderables from gltf file\n", (int)gltf_renderables.size());
        gltf_obj->add_renderable(gltf_renderables.data(),gltf_renderables.size());
    }
    
    // gltf_scene_init();
    // ///////// //
    // main loop //
    // ///////// //
    int cnt = 0;
    while (eng.render()) {

        // frame();
        cnt++;
    }

    eng.terminate();
    return 0;
}

