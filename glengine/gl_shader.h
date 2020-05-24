#pragma once

#include "gl_types.h"
#include "gl_context.h"
#include "math/vmath.h"

#include <string>

namespace glengine {

class Shader {
  public:
    GLuint vertex_shader_id = 0;
    GLuint fragment_shader_id = 0;
    GLuint program_id = 0;
    // uniforms
    GLint u_id = -1;         // object id
    GLint u_model = -1;      // model matrix
    GLint u_view = -1;       // view matrix
    GLint u_projection = -1; // projection matrix
    GLint u_color = -1;      // mesh color
    GLint u_light0_pos = -1; // light position

    Shader() {}
    Shader(const char *vs, const char *fs) { init(vs, fs); }

    void init(const char *vs_src, const char *fs_src);

    void activate();
    void deactivate();

    void set_uniform_id(ID id);
    void set_uniform_model(const math::Matrix4f &m);
    void set_uniform_view(const math::Matrix4f &v);
    void set_uniform_projection(const math::Matrix4f &p);
    void set_uniform_color(const Color &c);
    void set_uniform_color(const math::Vector4f &c);
    void set_uniform_light0_pos(const math::Vector3f &pos);

  private:
    GLuint create_shader(const char *source, GLenum shader_type);
    GLuint create_program();
    void print_shader_info_log(GLuint shader);
    void print_program_info_log(GLuint program);
    void update_uniforms();
};

} // namespace glengine
