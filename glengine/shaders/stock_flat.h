#pragma once

#include "stock_common.h"

#include <vector>

namespace {

const char *flat_fs_src =
    R"(
void main() {
    fragment_color = u_color;
})";

const char *flat_textured_fs_src =
    R"(
// additional uniforms not defined in the common part
uniform sampler2D tex_basecolor;

void main() {
    vec4 color = texture(tex_basecolor, tex_coord) * u_color;
    if(color.a < 0.1)
        discard;
    // output
    fragment_color = color;
})";


}

namespace glengine {
namespace stock_shaders {

// //// //
// flat //
// //// //

static std::vector<const char*> flat_vs_srcs = {common_vs_src, default_vs_src};
static std::vector<const char*> flat_fs_srcs = {common_fs_src, flat_fs_src};

// ///////////// //
// flat textured //
// ///////////// //

static std::vector<const char*> flat_textured_vs_srcs = {common_vs_src, default_vs_src};
static std::vector<const char*> flat_textured_fs_srcs = {common_fs_src, flat_textured_fs_src};
} //
} // 
