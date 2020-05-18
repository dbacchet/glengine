#pragma once

#include "gl_context.h"

#include <string>

namespace glengine {

class Shader {
  public:
    GLuint vertex_shader_id = 0;
    GLuint fragment_shader_id = 0;
    GLuint program_id = 0;

    Shader() {}
    Shader(const char *vs, const char *fs) { init(vs, fs); }

    void init(const char *vs_src, const char *fs_src);

  private:
    GLuint create_shader(const char *source, GLenum shader_type);
    GLuint create_program();
    void print_shader_info_log(GLuint shader);
    void print_program_info_log(GLuint program);
};

} // namespace glengine
