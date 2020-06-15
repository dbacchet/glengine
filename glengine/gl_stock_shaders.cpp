#include "gl_stock_shaders.h"

#include "shaders/stock_common.h"
#include "shaders/stock_diffuse.h"
#include "shaders/stock_flat.h"
#include "shaders/stock_phong.h"
#include "shaders/stock_quad.h"
#include "shaders/stock_ssao.h"
#include "shaders/stock_vertexcolor.h"


namespace glengine {

ShaderSrc get_stock_shader_source(StockShader type) {
    switch (type) {
    case StockShader::Flat:
        return {stock_shaders::flat_vs_srcs, stock_shaders::flat_fs_srcs};
    case StockShader::Diffuse:
        return {stock_shaders::diffuse_vs_srcs, stock_shaders::diffuse_fs_srcs};
    case StockShader::DiffuseTextured:
        return {stock_shaders::diffuse_textured_vs_srcs, stock_shaders::diffuse_textured_fs_srcs};
    case StockShader::Phong:
        return {stock_shaders::phong_vs_srcs, stock_shaders::phong_fs_srcs};
    case StockShader::Ssao:
        return {stock_shaders::ssao_vs_srcs, stock_shaders::ssao_fs_srcs};
    case StockShader::Quad:
        return {stock_shaders::quad_vs_srcs, stock_shaders::quad_fs_srcs};
    case StockShader::QuadDeferred:
        return {stock_shaders::quad_deferred_vs_srcs, stock_shaders::quad_deferred_fs_srcs};
    default: // use vertexcolor by default
        return {stock_shaders::vertexcolor_vs_srcs, stock_shaders::vertexcolor_fs_srcs};
    }
}

} // namespace glengine
