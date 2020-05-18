#pragma once

#include "math/vmath.h"
#include <cstdint>

namespace glengine {

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

struct Vertex {
    math::Vector3f pos;
    Color color;
    // math::Vector3f normal;
};

} //
