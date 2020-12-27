#include "sokol_gfx.h"
#include "sokol_time.h"
#include "sokol_gfx_imgui.h"
#include "sokol_imgui.h"

#include "math/vmath.h"
#include "shaders/generated/multipass-basic.glsl.h"

#include "imgui/imgui_impl_glfw.h"

#include "gl_types.h"
#include "gl_context.h"
#include "gl_engine.h"
#include "gl_pipelines.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"

struct Pass {
    sg_pass_action pass_action = {0};
    sg_pass_desc pass_desc = {0};
    sg_pass pass_id = {0};
};

static struct {
    struct {
        Pass pass;
        sg_pipeline pip;
        sg_bindings bind;
    } offscreen;
    struct {
        Pass pass;
        sg_pipeline pip;
        sg_bindings bind;
    } fsq;
} state;


/* called initially and when window size changes */
void create_offscreen_pass(int width, int height) {
    /* destroy previous resource (can be called for invalid id) */
    sg_destroy_pass(state.offscreen.pass.pass_id);
    sg_destroy_image(state.offscreen.pass.pass_desc.color_attachments[0].image);
    sg_destroy_image(state.offscreen.pass.pass_desc.depth_stencil_attachment.image);

    /* create offscreen rendertarget images and pass */
    const int offscreen_sample_count = 1;
    sg_image_desc color_img_desc = {.render_target = true,
                                    .width = width,
                                    .height = height,
                                    .sample_count = offscreen_sample_count,
                                    .min_filter = SG_FILTER_LINEAR,
                                    .mag_filter = SG_FILTER_LINEAR,
                                    .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                    .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                    .label = "color image"};
    sg_image_desc depth_img_desc = {.render_target = true,
                                    .width = width,
                                    .height = height,
                                    .pixel_format = SG_PIXELFORMAT_DEPTH,
                                    .sample_count = offscreen_sample_count,
                                    .min_filter = SG_FILTER_LINEAR,
                                    .mag_filter = SG_FILTER_LINEAR,
                                    .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                                    .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
                                    .label = "depth image"};
    state.offscreen.pass.pass_desc = (sg_pass_desc){.color_attachments =
                                                        {
                                                            [0].image = sg_make_image(&color_img_desc),
                                                        },
                                                    .depth_stencil_attachment.image = sg_make_image(&depth_img_desc),
                                                    .label = "offscreen pass"};
    state.offscreen.pass.pass_id = sg_make_pass(&state.offscreen.pass.pass_desc);

    /* also need to update the fullscreen-quad texture bindings */
    state.fsq.bind.fs_images[0] = state.offscreen.pass.pass_desc.color_attachments[0].image;
}

int main() {

    glengine::GLEngine eng;
    eng.init({1280, 720, true});

    eng._camera_manipulator.set_azimuth(0.5f).set_elevation(0.8f);

    sg_setup((sg_desc){0});
    stm_setup();
    // use sokol-imgui with all default-options
    simgui_desc_t simgui_desc = {};
    simgui_desc.dpi_scale = eng._context.window_state.framebuffer_size.x / eng._context.window_state.window_size.x;
    simgui_setup(&simgui_desc);

    glengine::Pipelines pipelines;
    pipelines.init();

    // create offscreen pass
    create_offscreen_pass(eng._context.window_state.window_size.x, eng._context.window_state.window_size.y);

    glengine::Mesh grid(101,"grid");
    auto grid_md = glengine::create_grid_data(50.0f);
    grid.init(grid_md.vertices, grid_md.indices);

    glengine::Mesh box(102,"box");
    auto box_md = glengine::create_box_data();
    box.init(box_md.vertices, box_md.indices);

    /* cube vertex buffer */
    // clang-format off
    glengine::Vertex vertices[] = {
        {{-1.0f, -1.0f, -1.0f},   {255,0,0,255}},
        {{ 1.0f, -1.0f, -1.0f},   {255,0,0,255}},
        {{ 1.0f,  1.0f, -1.0f},   {255,0,0,255}},
        {{-1.0f,  1.0f, -1.0f},   {255,0,0,255}},

        {{-1.0f, -1.0f,  1.0f},   {0,255,0,255}},
        {{ 1.0f, -1.0f,  1.0f},   {0,255,0,255}},
        {{ 1.0f,  1.0f,  1.0f},   {0,255,0,255}},
        {{-1.0f,  1.0f,  1.0f},   {0,255,0,255}},

        {{-1.0f, -1.0f, -1.0f},   {0,0,255,255}},
        {{-1.0f,  1.0f, -1.0f},   {0,0,255,255}},
        {{-1.0f,  1.0f,  1.0f},   {0,0,255,255}},
        {{-1.0f, -1.0f,  1.0f},   {0,0,255,255}},

        {{ 1.0f, -1.0f, -1.0f},   {255,128,0,255}},
        {{ 1.0f,  1.0f, -1.0f},   {255,128,0,255}},
        {{ 1.0f,  1.0f,  1.0f},   {255,128,0,255}},
        {{ 1.0f, -1.0f,  1.0f},   {255,128,0,255}},

        {{-1.0f, -1.0f, -1.0f},   {0,128,255,255}},
        {{-1.0f, -1.0f,  1.0f},   {0,128,255,255}},
        {{ 1.0f, -1.0f,  1.0f},   {0,128,255,255}},
        {{ 1.0f, -1.0f, -1.0f},   {0,128,255,255}},

        {{-1.0f,  1.0f, -1.0f},   {255,0,128,255}},
        {{-1.0f,  1.0f,  1.0f},   {255,0,128,255}},
        {{ 1.0f,  1.0f,  1.0f},   {255,0,128,255}},
        {{ 1.0f,  1.0f, -1.0f},   {255,0,128,255}}
    };
    // clang-format on
    sg_buffer vbuf =
        sg_make_buffer((sg_buffer_desc){.size = sizeof(vertices), .content = vertices, .label = "cube-vertices"});

    /* create an index buffer for the cube */
    // clang-format off
    uint32_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };
    // clang-format on
    sg_buffer ibuf = sg_make_buffer((sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER, .size = sizeof(indices), .content = indices, .label = "cube-indices"});

    // offscreen pipeline
    /* pass action for offscreen pass */
    state.offscreen.pass.pass_action =
        (sg_pass_action){.colors = {
                             [0] = {.action = SG_ACTION_CLEAR, .val = {0.1f, 0.1f, 0.1f, 1.0f}},
                         }};
    state.offscreen.pip = pipelines.get(glengine::GL_PIPELINE_VERTEXCOLOR_TRIANGLES_INDEXED);

    /* resource bindings for offscreen rendering */
    state.offscreen.bind = (sg_bindings){.vertex_buffers[0] = vbuf, .index_buffer = ibuf};

    // fulscreen quad rendering
    float quad_vertices[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    sg_buffer quad_vbuf = sg_make_buffer(
        (sg_buffer_desc){.size = sizeof(quad_vertices), .content = quad_vertices, .label = "quad vertices"});

    /* the pipeline object to render the fullscreen quad */
    state.fsq.pip =
        sg_make_pipeline((sg_pipeline_desc){.layout = {.attrs[ATTR_vs_fsq_pos].format = SG_VERTEXFORMAT_FLOAT2},
                                            .shader = sg_make_shader(fsq_shader_desc()),
                                            .primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP,
                                            .label = "fullscreen quad pipeline"});

    /* resource bindings to render a fullscreen quad */
    state.fsq.bind = (sg_bindings){.vertex_buffers[0] = quad_vbuf,
                                   .fs_images = {
                                       [SLOT_tex0] = state.offscreen.pass.pass_desc.color_attachments[0].image,
                                   }};

    uint64_t last_time = 0;

    sg_imgui_t sg_imgui;
    sg_imgui_init(&sg_imgui);

    while (eng.render()) {
        const double delta_time = stm_sec(stm_laptime(&last_time));

        sg_pass_action pass_action = {.colors[0] = {.action = SG_ACTION_CLEAR, .val = {0.1f, 0.1f, 0.1f, 1.0f}}};
        int cur_win_width = eng._context.window_state.window_size.x;
        int cur_win_height = eng._context.window_state.window_size.y;
        int cur_fb_width = eng._context.window_state.framebuffer_size.x;
        int cur_fb_height = eng._context.window_state.framebuffer_size.y;

        ImGui_ImplGlfw_NewFrame();
        simgui_new_frame(cur_fb_width, cur_fb_height, delta_time);

        sg_begin_pass(state.offscreen.pass.pass_id, &state.offscreen.pass.pass_action);
        sg_apply_pipeline(pipelines.get(glengine::GL_PIPELINE_VERTEXCOLOR_TRIANGLES_INDEXED));
        // first object
        sg_apply_bindings(&box.bind);
        vs_params_t vs_params;
        vs_params.view = eng._camera.inverse_transform();
        vs_params.projection = eng._camera.projection();
        vs_params.model = math::create_translation<float>({-1.0f, -1.0f, 0.0f});
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
        sg_draw(0, 36, 1);
        // second object
        sg_apply_bindings(&box.bind);
        vs_params.model = math::create_translation<float>({-1.0f, 1.0f, 0.0f});
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
        sg_draw(0, 36, 1);
        // third object
        sg_apply_bindings(&box.bind);
        vs_params.model = math::create_translation<float>({1.0f, 1.0f, 0.0f});
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
        sg_draw(0, 36, 1);
        // forth object
        sg_apply_bindings(&box.bind);
        vs_params.model = math::create_translation<float>({1.0f, -1.0f, 0.0f});
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
        sg_draw(0, 36, 1);
        // grid
        sg_apply_pipeline(pipelines.get(glengine::GL_PIPELINE_VERTEXCOLOR_LINES));
        sg_apply_bindings(&grid.bind);
        vs_params.model = math::create_translation<float>({0.0f, 0.0f, 0.0f});
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
        sg_draw(0, grid.vertices.size(), 1);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("sokol-gfx")) {
                ImGui::MenuItem("Buffers", 0, &sg_imgui.buffers.open);
                ImGui::MenuItem("Images", 0, &sg_imgui.images.open);
                ImGui::MenuItem("Shaders", 0, &sg_imgui.shaders.open);
                ImGui::MenuItem("Pipelines", 0, &sg_imgui.pipelines.open);
                ImGui::MenuItem("Passes", 0, &sg_imgui.passes.open);
                ImGui::MenuItem("Calls", 0, &sg_imgui.capture.open);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        sg_end_pass();
        // render to final framebuffer
        sg_begin_default_pass(&pass_action, cur_fb_width, cur_fb_height);
        sg_apply_pipeline(state.fsq.pip);
        sg_apply_bindings(&state.fsq.bind);
        // sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_fsq_params, &fsq_params, sizeof(fsq_params));
        sg_draw(0, 4, 1);
        sg_imgui_draw(&sg_imgui);
        simgui_render();
        sg_end_pass();
        // imgui
        sg_commit();
    }

    eng.terminate();
    return 0;
}

