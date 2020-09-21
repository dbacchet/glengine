#pragma once

#include "stock_common.h"

#include <vector>

namespace {

const char *vertexcolor_fs_src =
    R"(
void main() {
    fragment_color = vcolor * u_color; // mix global color with vertex color
})";


}

namespace glengine {
namespace stock_shaders {

// /////////// //
// vertexcolor //
// /////////// //

static std::vector<const char*> vertexcolor_vs_srcs = {common_vs_src, default_vs_src};
static std::vector<const char*> vertexcolor_fs_srcs = {common_fs_src, vertexcolor_fs_src};

} //
} // 
