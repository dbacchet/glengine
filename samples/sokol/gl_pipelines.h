#pragma once

#include "sokol_gfx.h"

namespace glengine {

enum gl_shaders { GL_SHADER_VERTEXCOLOR, GL_SHADERS_NUM };

// pipelines: combine specific shader and primitive
enum gl_pipelines {
    GL_PIPELINE_VERTEXCOLOR_POINTS,
    GL_PIPELINE_VERTEXCOLOR_LINES,
    GL_PIPELINE_VERTEXCOLOR_LINE_STRIP,
    GL_PIPELINE_VERTEXCOLOR_TRIANGLES,
    GL_PIPELINE_VERTEXCOLOR_TRIANGLE_STRIP,

    GL_PIPELINE_VERTEXCOLOR_POINTS_INDEXED,
    GL_PIPELINE_VERTEXCOLOR_LINES_INDEXED,
    GL_PIPELINE_VERTEXCOLOR_LINE_STRIP_INDEXED,
    GL_PIPELINE_VERTEXCOLOR_TRIANGLES_INDEXED,
    GL_PIPELINE_VERTEXCOLOR_TRIANGLE_STRIP_INDEXED,

    GL_PIPELINES_NUM
};

class Pipelines {
  public:
    /// create and initialize all the valid pipelines
    bool init() { return init_shaders() && init_pipelines(); }
    /// get a pipeline
    sg_pipeline get(gl_pipelines type) const { return pipelines[type]; }

  protected:

    bool init_shaders();
    bool init_pipelines();

    sg_shader shaders[GL_SHADERS_NUM] = {0};
    sg_pipeline pipelines[GL_PIPELINES_NUM] = {0};
};

} // namespace glengine
