#pragma once

#include "gl_types.h"
#include "gl_engine.h"
#include "sokol_gfx.h"

#include <string>

namespace glengine {

class EffectSSAO {
  public:
    EffectSSAO() {}
    virtual ~EffectSSAO() = default;

    virtual bool init(GLEngine &eng, sg_primitive_type primitive, sg_index_type idx_type = SG_INDEXTYPE_NONE);

    virtual void update_bindings();

    virtual void apply_uniforms();

    sg_image tex_depth = {0};
    sg_image tex_normal = {0};
    sg_image tex_random = {0};
    sg_pipeline pip = {0};
    sg_bindings bind = {0};

    float radius = 0.5f;
    float bias = 0.025f;
    float noise_scale = 1.0f;
    GLEngine *_eng;
};

} // namespace glengine
