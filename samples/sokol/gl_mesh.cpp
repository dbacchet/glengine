#include "gl_mesh.h"

// #include "gl_context.h"
#include "gl_types.h"

// #include "gl_shader.h"
// #include "gl_texture.h"

#include <vector>

namespace glengine {

Mesh::Mesh(ID id, const std::string &name)
: Resource(id, name) {}

bool Mesh::init(const std::vector<Vertex> &vertices_) {
    return init(vertices_, std::vector<uint32_t>());
}

bool Mesh::init(const std::vector<Vertex> &vertices_, const std::vector<uint32_t> &indices_) {
    vertices = vertices_;
    indices = indices_;
    setup_mesh();
    return true;
}

void Mesh::setup_mesh() {
    vbuf = sg_make_buffer((sg_buffer_desc){
        .size = int(vertices.size() * sizeof(Vertex)), .content = vertices.data(), .label = "mesh-vertices"});

    if (indices.size() > 0) {
        ibuf = sg_make_buffer((sg_buffer_desc){.type = SG_BUFFERTYPE_INDEXBUFFER,
                                               .size = int(indices.size() * sizeof(uint32_t)),
                                               .content = indices.data(),
                                               .label = "mesh-indices"});
    }
    bind = {.vertex_buffers[0] = vbuf, .index_buffer = ibuf};
}

// update the data in the buffers. buffers have to be already allocated
bool Mesh::update() {
    // glBindVertexArray(vao);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // GLint size = 0;
    // glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    // if (size > int(vertices.size() * sizeof(Vertex))) { // reuse existing memory
    //     glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), &vertices[0]);
    // } else { // reallocate the memory
    //     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    // }
    //
    // if (indices.size() > 0) {
    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    //     glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    //     if (size > int(indices.size() * sizeof(uint32_t))) { // reuse existing memory
    //         glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(uint32_t), &indices[0]);
    //     } else { // reallocate the memory
    //         glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);
    //     }
    // }
    // glBindVertexArray(0);
    return true;
}

void Mesh::update_bindings(sg_bindings &bind) {
    bind.vertex_buffers[0] = vbuf;
    bind.index_buffer = ibuf;
}

} // namespace glengine
