#pragma once

#include "gl_context.h" // this is needed for the GLhalf type

#include "math/vmath.h"
#include <cstdint>
#include <limits>

namespace glengine {

using ID = uint32_t; ///< id used for every object managed by the engine
constexpr ID NULL_ID = std::numeric_limits<ID>::max();

struct Color {
    uint8_t r = 100;
    uint8_t g = 100;
    uint8_t b = 100;
    uint8_t a = 255;
};

struct TexCoords {
    float s = 0;
    float t = 0;
    // GLhalf s = 0;
    // GLhalf t = 0;
};

struct Vertex {
    math::Vector3f pos;
    Color color;
    math::Vector3f normal = {0,0,1};
    TexCoords tex_coords;
};

} // namespace glengine
