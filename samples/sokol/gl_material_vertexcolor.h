#pragma once

#include "gl_material.h"

#include "sokol_gfx.h"

#include <string>

namespace glengine {

class MaterialVertexColor : public Material {
  public:
    MaterialVertexColor()
    : Material() {}
    virtual ~MaterialVertexColor() = default;

    virtual bool init(sg_primitive_type primitive, sg_index_type idx_type = SG_INDEXTYPE_NONE) override;

    virtual void update_bindings(sg_bindings &bind) override;
};

} // namespace glengine
