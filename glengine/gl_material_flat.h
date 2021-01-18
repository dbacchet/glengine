#pragma once

#include "gl_material.h"

#include "sokol_gfx.h"

#include <string>

namespace glengine {

class MaterialFlat : public Material {
  public:
    MaterialFlat()
    : Material() {}
    virtual ~MaterialFlat() = default;

    virtual bool init(GLEngine &eng, sg_primitive_type primitive, sg_index_type idx_type = SG_INDEXTYPE_NONE) override;

    virtual void update_bindings(sg_bindings &bind) override;

    virtual void apply_uniforms(const common_uniform_params_t &params) override;
};

class MaterialFlatTextured : public Material {
  public:
    MaterialFlatTextured()
    : Material() {
        color = {255, 255, 255, 255};
    }
    virtual ~MaterialFlatTextured() = default;

    virtual bool init(GLEngine &eng, sg_primitive_type primitive, sg_index_type idx_type = SG_INDEXTYPE_NONE) override;

    virtual void update_bindings(sg_bindings &bind) override;

    virtual void apply_uniforms(const common_uniform_params_t &params) override;

    sg_image tex_diffuse = {0};
};

} // namespace glengine
