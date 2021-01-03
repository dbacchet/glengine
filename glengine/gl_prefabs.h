#pragma once

#include "gl_types.h"

#include <vector>

namespace glengine {

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

MeshData create_axis_data();
MeshData create_quad_data();
MeshData create_box_data(const math::Vector3f &size={1.0f,1.0f,1.0f});
MeshData create_sphere_data(float radius, uint32_t subdiv=10);
MeshData create_grid_data(float len, float step=1.0f);

} // namespace glengine
