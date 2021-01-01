#pragma once

#include "gl_material.h"

#include "sokol_gfx.h"

#include <string>

namespace glengine {

class MaterialFlatTextured : public Material {
  public:
    MaterialFlatTextured()
    : Material() {}
    virtual ~MaterialFlatTextured() = default;

    virtual bool init(ResourceManager &rm, sg_primitive_type primitive, sg_index_type idx_type = SG_INDEXTYPE_NONE) override;

    virtual void update_bindings(sg_bindings &bind) override;

    virtual void apply_uniforms(const common_uniform_params_t &params) override;

    Color color = {180,180,180,255};
    sg_image tex_diffuse = {0};
};

} // namespace glengine
