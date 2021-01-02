#include "gl_mesh.h"

// #include "gl_context.h"
#include "gl_types.h"

// #include "gl_shader.h"
// #include "gl_texture.h"

#include <vector>

namespace glengine {

bool Mesh::init(const std::vector<Vertex> &vertices_, const std::vector<uint32_t> &indices_, sg_usage usage) {
    vertices = vertices_;
    indices = indices_;
    _usage = usage;
    setup_mesh();
    return true;
}

void Mesh::setup_mesh() {
    vbuf_size = int(vertices.size() * sizeof(Vertex));
    ibuf_size = int(indices.size() * sizeof(uint32_t));
    if (_usage == SG_USAGE_IMMUTABLE) {
        // init with info and content
        vbuf = sg_make_buffer((sg_buffer_desc){.type = SG_BUFFERTYPE_VERTEXBUFFER,
                                               .size = vbuf_size,
                                               .usage = _usage,
                                               .content = vertices.data(),
                                               .label = "mesh-vertices"});

        if (indices.size() > 0) {
            ibuf = sg_make_buffer((sg_buffer_desc){.type = SG_BUFFERTYPE_INDEXBUFFER,
                                                   .size = ibuf_size,
                                                   .usage = _usage,
                                                   .content = indices.data(),
                                                   .label = "mesh-indices"});
        }
    } else { // dynamic and streaming mesh buffers have to be declared and initialized in 2 steps
        vbuf = sg_make_buffer((sg_buffer_desc){.type = SG_BUFFERTYPE_VERTEXBUFFER,
                                               .size = vbuf_size,
                                               .usage = _usage,
                                               .content = nullptr,
                                               .label = "mesh-vertices"});

        if (indices.size() > 0) {
            ibuf = sg_make_buffer((sg_buffer_desc){.type = SG_BUFFERTYPE_INDEXBUFFER,
                                                   .size = ibuf_size,
                                                   .usage = _usage,
                                                   .content = nullptr,
                                                   .label = "mesh-indices"});
        }
        update();
    }
}

// update the data in the buffers. buffers have to be already allocated
bool Mesh::update() {
    // in case the new data is bigger than the actual buffers, create a bigger one
    int32_t new_vbuf_size = int32_t(vertices.size() * sizeof(Vertex));
    int32_t new_ibuf_size = int32_t(indices.size() * sizeof(uint32_t));
    if (new_vbuf_size > vbuf_size) {
        sg_destroy_buffer(vbuf);
        vbuf = sg_make_buffer((sg_buffer_desc){.type = SG_BUFFERTYPE_VERTEXBUFFER,
                                               .size = new_vbuf_size,
                                               .usage = _usage,
                                               .content = nullptr,
                                               .label = "mesh-vertices"});
        vbuf_size = new_vbuf_size;
    }
    if (new_ibuf_size > ibuf_size) {
        sg_destroy_buffer(ibuf);
        ibuf = sg_make_buffer((sg_buffer_desc){.type = SG_BUFFERTYPE_INDEXBUFFER,
                                               .size = new_ibuf_size,
                                               .usage = _usage,
                                               .content = nullptr,
                                               .label = "mesh-vertices"});
        ibuf_size = new_ibuf_size;
    }
    // update content
    sg_update_buffer(vbuf, vertices.data(), int(vertices.size() * sizeof(Vertex)));
    if (ibuf.id != SG_INVALID_ID) {
        sg_update_buffer(ibuf, indices.data(), int(indices.size() * sizeof(uint32_t)));
    }
    return true;
}

void Mesh::update_bindings(sg_bindings &bind) {
    bind.vertex_buffers[0] = vbuf;
    bind.index_buffer = ibuf;
}

} // namespace glengine
