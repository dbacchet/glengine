#pragma once

#include "gl_context.h" // this is needed for OpenGL types

#include "math/vmath.h"
#include <cstdint>
#include <limits>

namespace glengine {

using ID = uint32_t; ///< id used for every object managed by the engine
constexpr ID NULL_ID = std::numeric_limits<ID>::max();
constexpr GLuint INVALID_BUFFER = std::numeric_limits<GLuint>::max();

struct Color {
    uint8_t r = 180;
    uint8_t g = 180;
    uint8_t b = 180;
    uint8_t a = 255;
};

struct Vertex {
    math::Vector3f pos;
    Color color = {180, 180, 180, 255};
    math::Vector3f normal = {0, 0, 1};
    math::Vector2f tex_coords = {0, 0};
    math::Vector3f tangent = {1, 0, 0};
    math::Vector3f bitangent = {0, 1, 0};
    uint8_t padding[4] = {}; // padding to have a multiple of 16bytes
};

/// murmur hash function, 32bit, based on the code made by Austin Appleby (http://code.google.com/p/smhasher/)
uint32_t murmur_hash2_32(const void *key, int len, uint32_t seed = 0);
uint32_t murmur_hash2_32(const char *str, uint32_t seed = 0);

} // namespace glengine
