#pragma once

#include "gl_types.h"
#include "gl_context.h"
#include "math/vmath.h"

#include <string>

namespace glengine {

class Shader {
  public:
    ID id = NULL_ID; ///< id used in the engine
    GLuint vertex_shader_id = 0;
    GLuint fragment_shader_id = 0;
    GLuint program_id = 0; ///< shader program id in opengl
    // uniforms
    GLint u_id = -1;         // object id
    GLint u_model = -1;      // model matrix
    GLint u_view = -1;       // view matrix
    GLint u_projection = -1; // projection matrix
    GLint u_color = -1;      // mesh color
    GLint u_light0_pos = -1; // light position

    Shader(ID id_=NULL_ID)
    : id(id_) {}
    Shader(ID id_, const char *vs, const char *fs)
    : id(id_) {
        init(vs, fs);
    }

    void init(const char *vs_src, const char *fs_src);

    void activate();
    void deactivate();

    void set_uniform_id(ID id);
    void set_uniform_model(const math::Matrix4f &m);
    void set_uniform_view(const math::Matrix4f &v);
    void set_uniform_projection(const math::Matrix4f &p);
    void set_uniform_color(const Color &c);
    void set_uniform_light0_pos(const math::Vector3f &pos);

    // utility uniform functions
    void set_bool(const char *name, bool value) const;
    void set_int(const char *name, int value) const;
    void set_float(const char *name, float value) const;
    void set_vec2(const char *name, const math::Vector2f &value) const;
    void set_vec2(const char *name, float x, float y) const;
    void set_vec3(const char *name, const math::Vector3f &value) const;
    void set_vec3(const char *name, float x, float y, float z) const;
    void set_vec4(const char *name, const math::Vector4f &value) const;
    void set_vec4(const char *name, float x, float y, float z, float w);
    void set_mat3(const char *name, const math::Matrix3f &mat) const;
    void set_mat4(const char *name, const math::Matrix4f &mat) const;

  private:
    GLuint create_shader(const char *source, GLenum shader_type);
    GLuint create_program();
    void print_shader_info_log(GLuint shader);
    void print_program_info_log(GLuint program);
    void update_uniforms();
};

} // namespace glengine
