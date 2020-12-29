#pragma once

#include "gl_types.h"
#include "sokol_gfx.h"

#include <string>

namespace glengine {

class Material {
  public:

    virtual bool init(sg_primitive_type primitive, sg_index_type=SG_INDEXTYPE_NONE) = 0;

    virtual void update_bindings(sg_bindings &bind) {
        //
    }

    sg_pipeline pip = {0};
};

} // namespace glengine
