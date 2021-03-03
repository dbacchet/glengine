#pragma once

#include "math/vmath.h"

#include <cstdint>
#include <limits>
#include <string>

namespace glengine {

using ID = uint32_t; ///< id used for every object managed by the engine
constexpr ID NULL_ID = std::numeric_limits<ID>::max();

// struct Color {
//     uint8_t r = 180;
//     uint8_t g = 180;
//     uint8_t b = 180;
//     uint8_t a = 255;
// };
using Color = math::Vector4<uint8_t>;

struct Vertex {
    math::Vector3f pos;
    Color color = {180, 180, 180, 255};
    math::Vector3f normal = {0, 0, 1};
    math::Vector2f tex_coords = {0, 0};
    math::Vector3f tangent = {1, 0, 0};
};

// /// base class used for all the resources managed by the engine,
// /// for example shaders, textures, meshes, etc.
// class Resource {
//   protected:
//     Resource(ID id_, const std::string &name_)
//     : id(id_)
//     , name(name_) {}
//
//     ID id = NULL_ID;       ///< resource id. Unique per class of resource
//     std::string name = ""; ///< resource name. Not guaranteed to be unique
// };

struct common_uniform_params_t {
    math::Matrix4f model;
    math::Matrix4f view;
    math::Matrix4f projection;
};
} // namespace glengine
