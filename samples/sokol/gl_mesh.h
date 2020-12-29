#pragma once

#include "sokol_gfx.h"
#include "gl_types.h"

#include <string>
#include <vector>

namespace glengine {

/// Very simple mesh class, that can allocate and update the verted and (optionally) index data.
/// The class is _completely passive_ so after updating the vertices or indices data you have to explicitly
/// call the update() function.
class Mesh: public Resource {
  public:
    // mesh data
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    Mesh(ID id, const std::string &name);

    bool init(const std::vector<Vertex> &vertices_);
    bool init(const std::vector<Vertex> &vertices_, const std::vector<uint32_t> &indices_);
    // update the opengl buffers to reflect the vertices and indices arrays
    bool update();

    void update_bindings(sg_bindings &bind);

    sg_buffer vbuf = {0};
    sg_buffer ibuf = {SG_INVALID_ID};

  private:
    void setup_mesh();
};
} // namespace glengine
