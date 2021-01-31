#pragma once

#include "gl_types.h"
#include "gl_engine.h"
#include "sokol_gfx.h"

#include <string>

namespace glengine {

class EffectBlur {
  public:
    EffectBlur() {}
    virtual ~EffectBlur() = default;

    virtual bool init(GLEngine &eng, sg_primitive_type primitive, sg_index_type idx_type = SG_INDEXTYPE_NONE);

    virtual void update_bindings();

    virtual void apply_uniforms();

    sg_image tex = {0}; // input texture to filter
    sg_pipeline pip = {0};
    sg_bindings bind = {0};

    bool initialized = false;
    GLEngine *_eng;
};

} // namespace glengine
