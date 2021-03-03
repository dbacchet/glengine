#include "math/vmath.h"
#include "math/math_utils.h"

#include "gl_engine.h"
#include "gl_context_glfw.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"
#include "gl_material_diffuse.h"
#include "gl_material_vertexcolor.h"
#include "gl_material_pbr.h"
#include "gl_material_pbr_ibl.h"
#include "gl_renderable.h"
#include "gl_utils.h"
#include "imgui/imgui.h"

#include "cgltf/cgltf.h"

#include "cmdline.h"

namespace glengine {
std::vector<Renderable> create_from_gltf(GLEngine &eng, const char *filename);
}

glengine::Object *recursive_parse_node(glengine::GLEngine &eng, cgltf_node *n) {
    if (!n) {
        return nullptr;
    }

    auto *go = eng.create_object({eng.create_axis_mesh(0.15f), eng.create_material<glengine::MaterialVertexColor>(
                                                                        SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)});
    // get local trasform
    math::Matrix4f tf;
    cgltf_node_transform_local(n, tf.data);
    go->set_transform(tf);
    for (auto ci=0; ci<n->children_count; ci++) {
        go->add_child(recursive_parse_node(eng, n->children[ci]));
    }
    return go;
}

void print_node_hierarchy(cgltf_node *n, int32_t indent=0) {
    printf("%*s", indent, "");
    printf("node '%s'\n", n->name);
    if (n->skin) {
        printf("%*s", indent, "");
        printf("joints '%lu'\n", n->skin->joints_count);
        printf("%*s", indent, "");
        printf("skeleton root '%p' ('%s')\n", n->skin->skeleton, n->skin->skeleton->name);
        if (n->skin->joints_count>0)
            printf("%*s", indent, "");
        for (auto i=0; i<n->skin->joints_count; i++) {
            printf("%s ", n->skin->joints[i]->name);
            printf("%lu | ", n->skin->inverse_bind_matrices->count);
        }
        if (n->skin->joints_count>0)
            printf("\n");
    }
    for (auto ci=0; ci<n->children_count; ci++) {
        print_node_hierarchy(n->children[ci], indent+2);
    }
}

template <typename T=float, typename U=float>
class AnimationChannel {
public:
    U get(T t) {
        t = math::utils::clamp(t, _min, _max);
        return math::utils::interpolate_linear(_t.data(), _data.data(), _t.size(), t);
    }

    T _min = std::numeric_limits<T>::lowest(); ///< min t value for animation clamp
    T _max = std::numeric_limits<T>::max(); ///< max t value for animation clamp
    std::vector<T> _t;
    std::vector<U> _data;
};

class AnimationData {
public:

    AnimationChannel<float, math::Vector3f> _position;
};

int main(int argc, char *argv[]) {

    cmdline::parser cl;
    cl.add<uint32_t>("width", 'w', "window width", false, 1280, cmdline::range(16, 65535));
    cl.add<uint32_t>("height", 'h', "window height", false, 720, cmdline::range(16, 65535));
    cl.add<float>("scaling", 's', "model scaling", false, 1.0f);
    cl.add("mrt", 'm', "use MRT and enable effects");
    cl.add("novsync", 'n', "disable vsync");
    cl.footer("filename ...");
    cl.parse_check(argc, argv);

    uint32_t width = cl.get<uint32_t>("width");
    uint32_t height = cl.get<uint32_t>("height");
    float scale = cl.get<float>("scaling");
    bool vsync = !cl.exist("novsync");
    bool use_mrt = cl.exist("mrt");

    // create context and engine
    glengine::ContextGLFW context;
    context.init({.window_width = width, .window_height = height, .vsync = vsync});
    glengine::GLEngine eng;
    eng.init(&context, {.use_mrt = use_mrt});

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
    for (std::string gltf_filename : cl.rest()) {
        gltf_obj = eng.create_object();
        auto gltf_renderables = glengine::create_from_gltf(eng, gltf_filename.c_str());
        printf("loaded %d renderables from gltf file\n", (int)gltf_renderables.size());
        gltf_obj->add_renderable(gltf_renderables.data(), gltf_renderables.size());

        // approximate camera placement using object extent
        auto aabb = glengine::calc_bounding_box(gltf_obj, true);
        printf("object bbox - center (%f,%f,%f) - size (%f,%f,%f)\n", aabb.center.x, aabb.center.y, aabb.center.z,
               aabb.size.x, aabb.size.y, aabb.size.z);
        eng._camera_manipulator.set_center(aabb.center);
        eng._camera_manipulator.set_distance(1.5f * math::length(aabb.size));

        if (cl.exist("scaling")) {
            gltf_obj->set_scale({scale, scale, scale});
            eng._camera_manipulator.center() *= scale;
            eng._camera_manipulator.distance() *= scale;
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
            ImGui::DragFloat("azimuth", &azimuth, 0.01, -2 * M_PI, 2 * M_PI);
            ImGui::DragFloat("elevation", &elevation, 0.01, 0, M_PI);
            ImGui::DragFloat("distance", &distance, 0.01, 0, 1000.0);
            ImGui::End();
        });
    }

    // load skeleton from the first model
    cgltf_options options = {};
    cgltf_data *data = NULL;
    std::string gltf_file = cl.rest()[0];
    cgltf_result result = cgltf_parse_file(&options, gltf_file.c_str(), &data);
    if (result == cgltf_result_success) {
        cgltf_load_buffers(&options, data, gltf_file.c_str());
        // scenes
        printf("the model has %lu scenes\n",data->scenes_count);
        for (auto si=0; si<data->scenes_count; si++) {
            const auto s = data->scenes[si];
            printf("scene %s, (%p) with %lu nodes\n", s.name, &s, s.nodes_count);
        }
        printf("default scene %s, (%p) with %lu nodes\n\n", data->scene->name, data->scene, data->scene->nodes_count);
        
        // // nodes
        // printf("the model has %lu total nodes\n",data->nodes_count);
        // for (auto ni=0; ni<data->nodes_count; ni++) {
        //     const auto &n = data->nodes[ni];
        //     printf("node %d (%p): name=%s mesh=%p, skin=%p\n", ni, &n, n.name, n.mesh, n.skin);
        //     if (n.children_count>0) {
        //         printf("   children: ");
        //         for (auto ci=0; ci<n.children_count; ci++) {
        //             printf("%p ", n.children[ci]);
        //         }
        //         printf("\n");
        //     }
        // }
        // node hierarchy (print and also create objects)
        for (auto ni = 0; ni<data->scene->nodes_count; ni++) {
            print_node_hierarchy(data->scene->nodes[ni]);
            auto *go = recursive_parse_node(eng, data->scene->nodes[ni]);
        }
        // animations
        for (auto na = 0; na<data->animations_count; na++) {
            const auto &anim = data->animations[na];
            printf("Animation: %s\n - channels: %lu, samplers: %lu\n", anim.name, anim.channels_count, anim.samplers_count);
            for (auto nc=0; nc<anim.channels_count; nc++) {
                printf(" channel %d - target node: %p\n", nc, anim.channels[nc].target_node);
            }
            for (auto ns=0; ns<anim.samplers_count; ns++) {
                printf(" sampler %d - input: %p output: %p\n", ns, anim.samplers[ns].input, anim.samplers[ns].output);
            }
            // print info for the first sampler input
            const auto &sinput = anim.samplers[0].input;
            printf("input type: %d %d\n", sinput->component_type, sinput->type);
            float *ibd = (float*)((char*)sinput->buffer_view->buffer->data + sinput->buffer_view->offset);
            for (int i=0; i<sinput->count; i++) {
                printf("%f ", ibd[i]);
            }
            printf("\n");
            // print info for the first sampler output
            const auto &soutput = anim.samplers[0].output;
            printf("output type: %d %d\n", soutput->component_type, soutput->type);
            float *obd = (float*)((char*)soutput->buffer_view->buffer->data + soutput->buffer_view->offset);
            for (int i=0; i<soutput->count; i++) {
                printf("%f ", obd[i]);
            }
            printf("\n");
        }
        
        // clean up
        cgltf_free(data);
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

