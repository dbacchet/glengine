#pragma once

#include "stock_common.h"

#include <vector>

namespace {

const char *vertexcolor_vs_src =
    R"(
void main() {
    frag_pos = vec3(u_view * u_model * vec4(v_position,1.0));       // fragment position in view-space
    normal = mat3(transpose(inverse(u_view * u_model))) * v_normal; // normal in view space - transp(inv()) is needed to take into account the scaling
    light_pos = vec3(u_view * vec4(u_light0_pos, 1.0));             // world-space light position to view-space light position
    tex_coord = v_texcoord0;
    vcolor   = v_color;
    gl_Position = u_projection * u_view * u_model * vec4(v_position,1.0);
    gl_PointSize = 2.0;
})";

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

static std::vector<const char*> vertexcolor_vs_srcs = {common_vs_src, vertexcolor_vs_src};
static std::vector<const char*> vertexcolor_fs_srcs = {common_fs_src, vertexcolor_fs_src};

} //
} // 
