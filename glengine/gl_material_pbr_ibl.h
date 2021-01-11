#pragma once

#include "gl_material.h"

#include "sokol_gfx.h"

#include <string>

namespace glengine {

class MaterialPBRIBL : public Material {
  public:
    MaterialPBRIBL()
    : Material() {
        color = {255, 255, 255, 255};
    }
    virtual ~MaterialPBRIBL() = default;

    virtual bool init(ResourceManager &rm, sg_primitive_type primitive,
                      sg_index_type idx_type = SG_INDEXTYPE_NONE) override;

    virtual void update_bindings(sg_bindings &bind) override;

    virtual void apply_uniforms(const common_uniform_params_t &params) override;


    math::Vector3f emissive_factor = {0.0f,0.0f,0.0f};
    float metallic_factor = 1.0f;
    float roughness_factor = 1.0f;
    sg_image tex_diffuse = {0};
    sg_image tex_metallic_roughness = {0};
    sg_image tex_normal = {0};
    sg_image tex_occlusion = {0};
    sg_image tex_emissive = {0};
};

} // namespace glengine
