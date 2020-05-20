#pragma once

#include "gl_types.h"

#include <vector>

namespace glengine {
enum class Prefab {
    Axis = 0,
    Box,
    Quad,
    Sphere,
    Grid,
    NumPrefabs
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

MeshData create_box_data(const math::Vector3f &size={1.0f,1.0f,1.0f});
MeshData create_grid_data(float len, float step=1.0f);

} // namespace glengine
