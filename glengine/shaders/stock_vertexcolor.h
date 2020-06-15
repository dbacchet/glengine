#pragma once

#include "stock_common.h"

#include <vector>

namespace {

const char *vertexcolor_fs_src =
    R"(
void main() {
    fragment_color = vec4(normalize(vec3(light_pos)),1);//vcolor;
    fragment_color = vcolor;
    object_id = u_id;
    // for deferred rendering
    g_position = vec4(frag_pos,1.0);
    g_normal = vec4(normalize(light_pos),1.0); // alignign the normal with the light will make the color flat
    g_albedospec.rgb = vcolor.rgb;
    g_albedospec.a = vcolor.a;
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
