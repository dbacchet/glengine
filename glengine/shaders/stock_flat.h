#pragma once

#include "stock_common.h"

#include <vector>

namespace {

const char *flat_fs_src =
    R"(
void main() {
    fragment_color = u_color;
    object_id = u_id;
    // for deferred rendering
    g_position = vec4(frag_pos,1.0);
    g_normal = vec4(normalize(light_pos),1.0); // alignign the normal with the light will make the color flat
    g_albedospec.rgb = u_color.rgb;
    g_albedospec.a = u_color.a;
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
