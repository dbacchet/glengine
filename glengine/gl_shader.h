#pragma once

#include "gl_types.h"
#include "gl_context.h"
#include "math/vmath.h"

#include <string>
#include <unordered_map>

namespace glengine {

class Shader {
  public:
    struct Uniform {
        GLint location = -1; ///< location of the uniform
        GLenum type = 0;     ///< type of the uniform variable in the program
    };

  public:
    ID id = NULL_ID; ///< id used in the engine
    std::string name = "";
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

    std::unordered_map<std::string, Uniform> _uniforms;

    Shader(ID id_ = NULL_ID, const std::string &name_="_unnamed_")
    : id(id_), name(name_) {}
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
    bool has_uniform(const char *name) const;
    void set_bool(const char *name, bool value) const;
    void set_int(const char *name, int value) const;
    void set_float(const char *name, float value) const;
    void set_vec2(const char *name, const math::Vector2f &value) const;
    void set_vec3(const char *name, const math::Vector3f &value) const;
    void set_vec4(const char *name, const math::Vector4f &value) const;
    void set_mat3(const char *name, const math::Matrix3f &mat) const;
    void set_mat4(const char *name, const math::Matrix4f &mat) const;
    void set_sampler(const char *name, int value) const;

  private:
    GLuint create_shader(const char *source, GLenum shader_type);
    GLuint create_program();
    void print_shader_info_log(GLuint shader);
    void print_program_info_log(GLuint program);
    void extract_uniforms();
    void update_uniforms();

    bool validate_set_uniform_call(const char *name, GLenum type) const;
};

} // namespace glengine
