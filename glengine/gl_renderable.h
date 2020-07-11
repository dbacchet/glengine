#pragma once

#include "math/vmath.h"
#include "gl_types.h"

#include <memory>

namespace glengine {

class Mesh;
class Material;

struct Renderable {
    Mesh *mesh = nullptr;
    Material *material = nullptr;
};

} // namespace glengine
