#include "gl_texture.h"
#include "gl_context.h"

#include <string>

namespace glengine {

bool Texture::init(uint32_t width, uint32_t height, const uint8_t *data, GLint format, GLenum from_format) {
    if (!data) {
        // no valid data
        return false;
    }
    if (texture_id == NULL_TEXTURE_ID) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    // set the texture_id wrapping/filtering options (on the currently bound texture_id object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, from_format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return true;
}

} // namespace glengine
