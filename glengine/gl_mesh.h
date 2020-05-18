#pragma once

#include "gl_context.h"
#include "gl_types.h"

#include <vector>

namespace glengine {

/// Very simple mesh class in OpenGL, that can allocate and update the verted and (optionally) index data.
/// The class is _completely passive_ so after updating the vertices or indices data you have to explicitly
/// call the update() function.
class Mesh {
  public:
    // mesh data
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    GLenum primitive = GL_TRIANGLES;

    Mesh() {}

    bool init(const std::vector<Vertex> &vertices_, GLenum primitive_) {
        return init(vertices_, std::vector<uint32_t>(), primitive_);
    }

    bool init(const std::vector<Vertex> &vertices_, const std::vector<uint32_t> &indices_, GLenum primitive_) {
        if (vao!=0) {
            // mesh was already allocated
            return false;
        }
        vertices = vertices_;
        indices = indices_;
        primitive = primitive_;
        setup_mesh();
        return true;
    }

    bool update() {
        if (vao==0) {
            // mesh _not_ already allocated
            return false;
        }
        update_mesh_data();
        return true;
    }

    void draw(GLuint shader) {
        // draw mesh
        glUseProgram(shader);
        glBindVertexArray(vao);
        if (indices.size() > 0) {
            glDrawElements(primitive, indices.size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(primitive, 0, vertices.size());
        }
        glBindVertexArray(0);
    }

  private:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;

    void setup_mesh() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        if (indices.size() > 0) {
            glGenBuffers(1, &ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);
        }

        // Attributes use fixed location in glengine to make the mesh setup independent from the specific shader;
        // all shaders use conventional locations for the vertex attributes.
        // In case there is a strong coupling between attribute location and
        //  glUseProgram(shader);
        //  GLuint vpos_location = glGetAttribLocation(shader, "vPos");
        //  glEnableVertexAttribArray(vpos_location);
        //  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

        constexpr GLuint vpos_location = 0; // position
        constexpr GLuint vcol_location = 1; // color
        // vertex positions
        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        // vertex color
        glEnableVertexAttribArray(vcol_location);
        glVertexAttribPointer(vcol_location, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                              (void *)offsetof(Vertex, color));

        glBindVertexArray(0);
    }

    // update the data in the buffers. buffers have to be already allocated
    void update_mesh_data() {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        GLint size = 0;
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
        if (size>vertices.size()*sizeof(Vertex)) { // reuse existing memory
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), &vertices[0]); 
        } else { // reallocate the memory
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        }

        if (indices.size() > 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
            if (size>indices.size()*sizeof(uint32_t)) { // reuse existing memory
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(uint32_t), &indices[0]); 
            } else { // reallocate the memory
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);
            }
        }
        glBindVertexArray(0);
    }
};
} // namespace glengine
