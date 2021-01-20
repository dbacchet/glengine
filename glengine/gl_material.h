#pragma once

#include "gl_types.h"
#include "sokol_gfx.h"

#include <string>

namespace glengine {

class GLEngine;

class Material {
  public:
    virtual ~Material() = default;

    virtual bool init(GLEngine &eng, sg_primitive_type primitive, sg_index_type = SG_INDEXTYPE_NONE) = 0;

    virtual void update_bindings(sg_bindings &bind) {}

    virtual void apply_uniforms(const common_uniform_params_t &params) {}

    template <typename T>
        T* as() { return this; }

    Color color = {180,180,180,255};
    sg_pipeline pip = {0};
};

} // namespace glengine
