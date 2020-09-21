#pragma once

#include "stock_common.h"

#include <vector>

namespace {

const char *flat_fs_src =
    R"(
void main() {
    fragment_color = u_color;
})";


}

namespace glengine {
namespace stock_shaders {

// //// //
// flat //
// //// //

static std::vector<const char*> flat_vs_srcs = {common_vs_src, default_vs_src};
static std::vector<const char*> flat_fs_srcs = {common_fs_src, flat_fs_src};

} //
} // 
