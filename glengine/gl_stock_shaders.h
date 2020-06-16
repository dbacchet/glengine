#pragma once

#include <vector>

namespace glengine {
enum class StockShader {
    Flat = 0,
    Diffuse,
    DiffuseTextured,
    Phong,
    VertexColor,
    Ssao,
    QuadDebug,
    QuadDeferred,
    NumStockShader
};

struct ShaderSrc {
    std::vector<const char*> vertex_shader_srcs;
    std::vector<const char*> fragment_shader_srcs;
};

ShaderSrc get_stock_shader_source(StockShader type);

} // namespace glengine
