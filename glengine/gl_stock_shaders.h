#pragma once

namespace glengine {
enum class StockShader {
    Flat = 0,
    Diffuse,
    Phong,
    VertexColor,
    NumStockShader
};

struct ShaderSrc {
    const char *vertex_shader = nullptr;
    const char *fragment_shader = nullptr;
};

ShaderSrc get_stock_shader_source(StockShader type);

} // namespace glengine
