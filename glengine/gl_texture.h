#pragma once

#include "gl_types.h"

#include <string>
#include <limits>

namespace glengine {

constexpr GLuint NULL_TEXTURE_ID = std::numeric_limits<GLuint>::max();

struct Texture {
    ID id = NULL_ID; ///< id used in the engine
    std::string name = "";

    GLuint texture_id = NULL_TEXTURE_ID; ///< internal id used by opengl

    Texture(ID id_ = NULL_ID, const std::string &name_ = "_unnamed_")
    : id(id_), name(name_) {}
    bool init(uint32_t width, uint32_t height, const uint8_t *data, GLint format = GL_RGBA,
              GLenum from_format = GL_RGBA);
};

} // namespace glengine
