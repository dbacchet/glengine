#pragma once

#include "gl_context.h" // this is needed for the GLhalf type

#include "math/vmath.h"
#include <cstdint>

namespace glengine {

struct Color {
    uint8_t r = 100;
    uint8_t g = 100;
    uint8_t b = 100;
    uint8_t a = 255;
};

struct TexCoords {
    GLhalf s = 0;
    GLhalf t = 0;
};

struct Vertex {
    math::Vector3f pos;
    Color color;
    math::Vector3f normal = {0,0,1};
    TexCoords tex_coords;
};

} // namespace glengine
