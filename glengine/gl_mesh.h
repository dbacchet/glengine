#pragma once

#include "gl_types.h"
#include "gl_shader.h"
#include "gl_texture.h"

#include <vector>

namespace glengine {

/// Very simple mesh class in OpenGL, that can allocate and update the verted and (optionally) index data.
/// The class is _completely passive_ so after updating the vertices or indices data you have to explicitly
/// call the update() function.
class Mesh {
    struct Textures {
        Texture *diffuse = nullptr;
    };
  public:
    std::string name = "";
    // mesh data
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    GLenum primitive = GL_TRIANGLES;

    Textures textures;

    Mesh(const std::string &name_);

    bool init(const std::vector<Vertex> &vertices_, GLenum primitive_);
    bool init(const std::vector<Vertex> &vertices_, const std::vector<uint32_t> &indices_, GLenum primitive_);
    // update the opengl buffers to reflect the vertices and indices arrays
    bool update();

    void draw(Shader &shader);

  private:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;

    void setup_mesh();
    // update the data in the buffers. buffers have to be already allocated
    void update_mesh_data();

    void bind_textures(Shader &shader);
};
} // namespace glengine
