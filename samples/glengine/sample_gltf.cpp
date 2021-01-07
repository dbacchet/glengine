#include "math/vmath.h"

#include "gl_engine.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"
#include "gl_material_diffuse.h"
#include "gl_material_flat.h"
#include "gl_material_flat_textured.h"
#include "gl_material_vertexcolor.h"
#include "gl_renderable.h"

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

    eng._camera_manipulator.set_azimuth(0.3f).set_elevation(1.0f).set_distance(5.0f);

    // /////// //
    // objects //
    // /////// //
    // grid
    auto *grid = eng.create_object({eng.create_grid_mesh(100.0f, 2.0f),
                                    eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES)});

    // load a gltf file if passed in the command line
    if (gltf_filename != "") {
        auto *gltf_obj = eng.create_object();
        auto gltf_renderables = glengine::create_from_gltf(eng, gltf_filename.c_str());
        printf("loaded %d renderables from gltf file\n", (int)gltf_renderables.size());
        gltf_obj->add_renderable(gltf_renderables.data(),gltf_renderables.size());
    }
    
    // ///////// //
    // main loop //
    // ///////// //
    int cnt = 0;
    while (eng.render()) {
        cnt++;
    }

    eng.terminate();
    return 0;
}

