#include "gl_pipelines.h"
#include "gl_utils.h"

#include "math/vmath_types.h"

#include "sokol_gfx.h"

#include "gl_types.h"
#include "shaders/generated/multipass-basic.glsl.h"

namespace glengine {

bool Pipelines::init_shaders() {
    shaders[GL_SHADER_VERTEXCOLOR] = sg_make_shader(offscreen_vertexcolor_shader_desc());
    return true;
}

bool Pipelines::init_pipelines() {
    // using vertexcolor shader
    const int offscreen_sample_count = sg_query_features().msaa_render_targets ? 4:1;
    auto make_vertexcolor_pipeline = [&](sg_primitive_type primitive, sg_index_type index_type=SG_INDEXTYPE_NONE) {
        sg_pipeline_desc pip_desc = {
            .layout = {.buffers[0].stride = sizeof(Vertex),
                       .attrs = {[ATTR_vs_vertexcolor_position].format = SG_VERTEXFORMAT_FLOAT3,
                                 [ATTR_vs_vertexcolor_color0].format = SG_VERTEXFORMAT_UBYTE4N}},
            .shader = shaders[GL_SHADER_VERTEXCOLOR],
            .primitive_type = primitive,
            .index_type = index_type,
            .depth_stencil = {.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL, .depth_write_enabled = true},
            .blend = {.color_attachment_count = 1, .depth_format = SG_PIXELFORMAT_DEPTH},
            .rasterizer = {.cull_mode = SG_CULLMODE_NONE, .sample_count = offscreen_sample_count},
            .label = "offscreen pipeline"};
        printf("%20u | %llu\n",murmur_hash2_32(&pip_desc, sizeof(pip_desc), 12345678), murmur_hash2_64(&pip_desc, sizeof(pip_desc), 12345678));
        return sg_make_pipeline(pip_desc);
    };
    // non indexed
    pipelines[GL_PIPELINE_VERTEXCOLOR_POINTS] = make_vertexcolor_pipeline(SG_PRIMITIVETYPE_POINTS);
    pipelines[GL_PIPELINE_VERTEXCOLOR_LINES] = make_vertexcolor_pipeline(SG_PRIMITIVETYPE_LINES);
    pipelines[GL_PIPELINE_VERTEXCOLOR_LINE_STRIP] = make_vertexcolor_pipeline(SG_PRIMITIVETYPE_LINE_STRIP);
    pipelines[GL_PIPELINE_VERTEXCOLOR_TRIANGLES] = make_vertexcolor_pipeline(SG_PRIMITIVETYPE_TRIANGLES);
    pipelines[GL_PIPELINE_VERTEXCOLOR_TRIANGLE_STRIP] = make_vertexcolor_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP);
    // indexed
    pipelines[GL_PIPELINE_VERTEXCOLOR_POINTS_INDEXED] = make_vertexcolor_pipeline(SG_PRIMITIVETYPE_POINTS, SG_INDEXTYPE_UINT32);
    pipelines[GL_PIPELINE_VERTEXCOLOR_LINES_INDEXED] = make_vertexcolor_pipeline(SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_UINT32);
    pipelines[GL_PIPELINE_VERTEXCOLOR_LINE_STRIP_INDEXED] = make_vertexcolor_pipeline(SG_PRIMITIVETYPE_LINE_STRIP, SG_INDEXTYPE_UINT32);
    pipelines[GL_PIPELINE_VERTEXCOLOR_TRIANGLES_INDEXED] = make_vertexcolor_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
    pipelines[GL_PIPELINE_VERTEXCOLOR_TRIANGLE_STRIP_INDEXED] = make_vertexcolor_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_INDEXTYPE_UINT32);

    return true;
}

} // namespace glengine
