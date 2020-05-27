#include "gl_prefabs.h"

#include <vector>
#include <cmath>

namespace {

bool create_box(const math::Vector3f &size, std::vector<glengine::Vertex> &out_vertices, std::vector<uint32_t> &out_indices) {
    float a = size.x/2.0f;
    float b = size.y/2.0f;
    float c = size.z/2.0f;

    std::vector<glengine::Vertex> vertices = { {{-a,-b, c}, {50,50,250,255}, { 0, 0, 1}, {0,0}},   // top
                                               {{ a,-b, c}, {50,50,250,255}, { 0, 0, 1}, {1,0}},
                                               {{ a, b, c}, {50,50,250,255}, { 0, 0, 1}, {1,1}},
                                               {{-a, b, c}, {50,50,250,255}, { 0, 0, 1}, {0,1}},
                                               {{-a,-b,-c}, {50,50,250,255}, { 0, 0,-1}, {0,1}},   // bottom
                                               {{ a,-b,-c}, {50,50,250,255}, { 0, 0,-1}, {1,1}},
                                               {{ a, b,-c}, {50,50,250,255}, { 0, 0,-1}, {1,0}},
                                               {{-a, b,-c}, {50,50,250,255}, { 0, 0,-1}, {0,0}},
                                               {{ a,-b,-c}, {250,50,50,255}, { 1, 0, 0}, {0,0}},   // front
                                               {{ a, b,-c}, {250,50,50,255}, { 1, 0, 0}, {1,0}},
                                               {{ a, b, c}, {250,50,50,255}, { 1, 0, 0}, {1,1}},
                                               {{ a,-b, c}, {250,50,50,255}, { 1, 0, 0}, {0,1}},
                                               {{-a,-b,-c}, {250,50,50,255}, {-1, 0, 0}, {1,0}},   // back
                                               {{-a, b,-c}, {250,50,50,255}, {-1, 0, 0}, {0,0}},
                                               {{-a, b, c}, {250,50,50,255}, {-1, 0, 0}, {1,1}},
                                               {{-a,-b, c}, {250,50,50,255}, {-1, 0, 0}, {0,1}},
                                               {{ a, b,-c}, {50,250,50,255}, { 0, 1, 0}, {0,0}},   // left
                                               {{-a, b,-c}, {50,250,50,255}, { 0, 1, 0}, {1,0}},
                                               {{-a, b, c}, {50,250,50,255}, { 0, 1, 0}, {1,1}},
                                               {{ a, b, c}, {50,250,50,255}, { 0, 1, 0}, {0,1}},
                                               {{-a,-b,-c}, {50,250,50,255}, { 0,-1, 0}, {0,1}},   // right
                                               {{ a,-b,-c}, {50,250,50,255}, { 0,-1, 0}, {1,1}},
                                               {{ a,-b, c}, {50,250,50,255}, { 0,-1, 0}, {1,0}},
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

uint8_t srgb_i(uint8_t v) {
    float val = std::pow(float(v)/255,2.2f);
    return uint8_t(val*255);
}
std::vector<glengine::Vertex> create_grid(float len, float step) {
    std::vector<glengine::Vertex> vertices;
    const int32_t major = 5;
    int32_t nlines = len / step;
    glengine::Color col_x = {120, 20, 20, 255};
    glengine::Color col_y = {20, 120, 20, 255};
    glengine::Color col1 = {40, 40, 40, 255};
    glengine::Color col2 = {70, 70, 70, 255};
    col_x = {srgb_i(120), srgb_i(20), srgb_i(20), 255};
    col_y = {srgb_i(20), srgb_i(120), srgb_i(20), 255};
    col1 = {srgb_i(40), srgb_i(40), srgb_i(40), 255};
    col2 = {srgb_i(70), srgb_i(70), srgb_i(70), 255};
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

bool create_sphere(float radius, uint32_t subdiv, std::vector<glengine::Vertex> &out_vertices, std::vector<uint32_t> &out_indices) {
    const int num_sectors = 2*subdiv;
    const int num_stacks = subdiv;
    out_vertices.clear();
    out_indices.clear();
    // vertices
    for (int i = 0; i <= num_stacks; ++i) {
        float stack_angle = M_PI / 2 - i * M_PI / num_stacks; // starting from pi/2 to -pi/2
        float xy = radius * std::cos(stack_angle);     // r * cos(u)
        float z = radius * std::sin(stack_angle);      // r * sin(u)
        // add (num_sectors+1) vertices per stack. the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= num_sectors; ++j) {
            float sector_angle = j * 2 * M_PI / num_sectors; // starting from 0 to 2pi
            math::Vector3f pos(xy * std::cos(sector_angle), xy * std::sin(sector_angle), z);
            math::Vector3f normal = math::normalized(pos);
            math::Vector2f texcoord{float(j) / num_sectors, float(i) / num_stacks};
            out_vertices.push_back({pos, {140, 140, 140, 255}, normal, texcoord});
        }
    }
    // indices
    for (int i = 0; i < num_stacks; ++i) {
        uint32_t k1 = i * (num_sectors + 1); // beginning of current stack
        uint32_t k2 = k1 + num_sectors + 1;  // beginning of next stack
        for (int j = 0; j < num_sectors; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            if (i != 0) {
                out_indices.push_back(k1);
                out_indices.push_back(k2);
                out_indices.push_back(k1 + 1);
            }
            if (i != (num_stacks - 1)) {
                out_indices.push_back(k1 + 1);
                out_indices.push_back(k2);
                out_indices.push_back(k2 + 1);
            }
        }
    }
    return true;
}

}

namespace glengine {

MeshData create_axis_data() {
    MeshData md;
    md.vertices = { {{0,0,0}, {255,0,0,255}},   // x0
                    {{1,0,0}, {255,0,0,255}},   // x1
                    {{0,0,0}, {0,255,0,255}},   // y0
                    {{0,1,0}, {0,255,0,255}},   // y1
                    {{0,0,0}, {0,0,255,255}},   // z0
                    {{0,0,1}, {0,0,255,255}} }; // z1
    return md;
}

MeshData create_quad_data() {
    MeshData md;
    //-1 1
    // *---*1 1
    // |\  |
    // | \ |
    // |  \|
    // *---*1-1
    //-1-1  
    // md.vertices = {{{ 0.0f, 1.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {0,1}}, // this variant will draw to the top right quarter of the viewport
    //                {{ 0.0f, 0.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {0,0}},
    //                {{ 1.0f, 0.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {1,0}},
    //                {{ 0.0f, 1.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {0,1}},
    //                {{ 1.0f, 0.0f,-0.0f}, {255,255,255,255}, { 0, 0, 1}, {1,0}},
    //                {{ 1.0f, 1.0f,-0.0f}, {255,255,255,255}, { 0, 0, 1}, {1,1}}, };
    md.vertices = {{{-1.0f, 1.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {0,1}},
                   {{-1.0f,-1.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {0,0}},
                   {{ 1.0f,-1.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {1,0}},
                   {{-1.0f, 1.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {0,1}},
                   {{ 1.0f,-1.0f,-0.0f}, {255,255,255,255}, { 0, 0,-1}, {1,0}},
                   {{ 1.0f, 1.0f,-0.0f}, {255,255,255,255}, { 0, 0,-1}, {1,1}}, };
    return md;
}

MeshData create_box_data(const math::Vector3f &size) {
    MeshData md;
    create_box(size, md.vertices, md.indices);
    return md;
}

MeshData create_sphere_data(float radius, uint32_t subdiv) {
    MeshData md;
    create_sphere(radius, subdiv, md.vertices, md.indices);
    return md;
}

MeshData create_grid_data(float len, float step) {
    MeshData md;
    md.vertices = create_grid(len, step);
    return md;
}

} // namespace glengine
