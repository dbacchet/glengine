#pragma once

#include "math/vmath.h"
#include "gl_types.h"
#include "sokol_gfx.h"

#include <memory>
#include <cassert>

namespace glengine {

class Mesh;
class Material;

struct Renderable {
    Mesh *mesh = nullptr;
    Material *material = nullptr;
    sg_bindings bind = {0};

    void update_bindings();

    void apply_pipeline();
    void apply_bindings();
    void apply_uniforms();
};

} // namespace glengine
