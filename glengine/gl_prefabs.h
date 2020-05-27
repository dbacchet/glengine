#pragma once

#include "gl_types.h"

#include <vector>

namespace glengine {

struct TextureData {
    int32_t width = -1;
    int32_t height = -1;
    uint8_t channels = 4;
    std::vector<uint8_t> data;
};
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    TextureData texture_diffuse;
};

MeshData create_axis_data();
MeshData create_quad_data();
MeshData create_box_data(const math::Vector3f &size={1.0f,1.0f,1.0f});
MeshData create_sphere_data(float radius, uint32_t subdiv=10);
MeshData create_grid_data(float len, float step=1.0f);

std::vector<MeshData> create_from_gltf(const char *filename);

} // namespace glengine
