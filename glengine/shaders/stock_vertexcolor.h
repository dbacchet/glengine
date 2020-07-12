#pragma once

#include "stock_common.h"

#include <vector>

namespace {

const char *vertexcolor_fs_src =
    R"(
void main() {
    object_id = u_id;
    vec4 color = vcolor * u_color; // mix global color with vertex color
    // for deferred rendering
    g_position = vec4(frag_pos,1.0);
    g_normal = vec4(normalize(light_pos),1.0); // alignign the normal with the light will make the color flat
    g_albedospec.rgb = color.rgb;
    g_albedospec.a = color.a;
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
