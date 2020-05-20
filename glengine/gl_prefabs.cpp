#include "gl_prefabs.h"

#include <vector>

namespace {

bool create_box(const math::Vector3f &size, std::vector<glengine::Vertex> &out_vertices, std::vector<uint32_t> &out_indices) {
    float a = size.x/2.0f;
    float b = size.y/2.0f;
    float c = size.z/2.0f;

    std::vector<glengine::Vertex> vertices = { {{-a,-b, c}, {50,50,250,255}, { 0, 0, 1}, {0,0}},   // top
                                               {{ a,-b, c}, {50,50,250,255}, { 0, 0, 1}, {0,0}},
                                               {{ a, b, c}, {50,50,250,255}, { 0, 0, 1}, {0,0}},
                                               {{-a, b, c}, {50,50,250,255}, { 0, 0, 1}, {0,0}},
                                               {{-a,-b,-c}, {50,50,250,255}, { 0, 0,-1}, {0,0}},   // bottom
                                               {{ a,-b,-c}, {50,50,250,255}, { 0, 0,-1}, {0,0}},
                                               {{ a, b,-c}, {50,50,250,255}, { 0, 0,-1}, {0,0}},
                                               {{-a, b,-c}, {50,50,250,255}, { 0, 0,-1}, {0,0}},
                                               {{ a,-b,-c}, {250,50,50,255}, { 1, 0, 0}, {0,0}},   // front
                                               {{ a, b,-c}, {250,50,50,255}, { 1, 0, 0}, {0,0}},
                                               {{ a, b, c}, {250,50,50,255}, { 1, 0, 0}, {0,0}},
                                               {{ a,-b, c}, {250,50,50,255}, { 1, 0, 0}, {0,0}},
                                               {{-a,-b,-c}, {250,50,50,255}, {-1, 0, 0}, {0,0}},   // back
                                               {{-a, b,-c}, {250,50,50,255}, {-1, 0, 0}, {0,0}},
                                               {{-a, b, c}, {250,50,50,255}, {-1, 0, 0}, {0,0}},
                                               {{-a,-b, c}, {250,50,50,255}, {-1, 0, 0}, {0,0}},
                                               {{ a, b,-c}, {50,250,50,255}, { 0, 1, 0}, {0,0}},   // left
                                               {{-a, b,-c}, {50,250,50,255}, { 0, 1, 0}, {0,0}},
                                               {{-a, b, c}, {50,250,50,255}, { 0, 1, 0}, {0,0}},
                                               {{ a, b, c}, {50,250,50,255}, { 0, 1, 0}, {0,0}},
                                               {{-a,-b,-c}, {50,250,50,255}, { 0,-1, 0}, {0,0}},   // right
                                               {{ a,-b,-c}, {50,250,50,255}, { 0,-1, 0}, {0,0}},
                                               {{ a,-b, c}, {50,250,50,255}, { 0,-1, 0}, {0,0}},
                                               {{-a,-b, c}, {50,250,50,255}, { 0,-1, 0}, {0,0}},
    };
    std::vector<uint32_t> indices = {0,1,2, 0,2,3,
                                     4,7,6, 4,6,5,
                                     8,9,10, 8,10,11,
                                     12,15,14, 12,14,13,
                                     16,17,18, 16,18,19,
                                     20,21,22, 20,22,23
    };
    out_vertices = vertices;
    out_indices = indices;
    return true;
}

std::vector<glengine::Vertex> create_grid(float len, float step) {
    std::vector<glengine::Vertex> vertices;
    const int32_t major = 5;
    int32_t nlines = len / step;
    glengine::Color col_x = {120, 20, 20, 255};
    glengine::Color col_y = {20, 120, 20, 255};
    glengine::Color col1 = {40, 40, 40, 255};
    glengine::Color col2 = {70, 70, 70, 255};
    // main axis
    vertices.push_back({{-len, 0, 0}, col_x});
    vertices.push_back({{len, 0, 0}, col_x});
    vertices.push_back({{0, -len, 0}, col_y});
    vertices.push_back({{0, len, 0}, col_y});
    // other lines
    for (int32_t i = 1; i <= nlines; i++) {
        vertices.push_back({{-len, i * step, 0}, i % major ? col1 : col2});
        vertices.push_back({{len, i * step, 0}, i % major ? col1 : col2});
        vertices.push_back({{-len, -i * step, 0}, i % major ? col1 : col2});
        vertices.push_back({{len, -i * step, 0}, i % major ? col1 : col2});
        vertices.push_back({{i * step, -len, 0}, i % major ? col1 : col2});
        vertices.push_back({{i * step, len, 0}, i % major ? col1 : col2});
        vertices.push_back({{-i * step, -len, 0}, i % major ? col1 : col2});
        vertices.push_back({{-i * step, len, 0}, i % major ? col1 : col2});
    }
    return vertices;
}

}

namespace glengine {
// enum class Prefab {
//     Axis = 0,
//     Box,
//     Quad,
//     Sphere,
//     Grid,
//     NumPrefabs
// };

// MeshData get_prefab_mesh(Prefab type) {
//     MeshData md;
//     switch (type) {
//     case Prefab::Axis:
//         break;
//     case Prefab::Box:
//         create_box({1.0f, 1.0f, 1.0f}, md.vertices, md.indices);
//         break;
//     case Prefab::Quad:
//         break;
//     case Prefab::Sphere:
//         break;
//     case Prefab::Grid:
//         break;
//     default:
//         break;
//     }
//     return md;
// }

MeshData create_box_data(const math::Vector3f &size) {
    MeshData md;
    create_box(size, md.vertices, md.indices);
    return md;
}

MeshData create_grid_data(float len, float step) {
    MeshData md;
    md.vertices = create_grid(len, step);
    return md;
}

} // namespace glengine
