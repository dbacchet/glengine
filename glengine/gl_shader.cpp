#include "gl_shader.h"

#include <unordered_map>
#include <iostream>

namespace {

std::unordered_map<GLenum, std::string> shader_type_map = {
    {GL_FRAGMENT_SHADER, "FRAGMENT_SHADER"},
    {GL_VERTEX_SHADER, "VERTEX_SHADER"},
};

const char *to_string(GLenum type) {
    switch (type) {
case GL_BOOL:
    return "bool";
case GL_INT:
    return "int";
case GL_UNSIGNED_INT:
    return "unsigned int";
case GL_FLOAT:
    return "float";
case GL_FLOAT_VEC2:
    return "vec2";
case GL_FLOAT_VEC3:
    return "vec3";
case GL_FLOAT_VEC4:
    return "vec4";
case GL_FLOAT_MAT3:
    return "mat3";
case GL_FLOAT_MAT4:
    return "mat4";
case GL_SAMPLER_2D:
    return "sampler2D";
default:
    return "???";
    }
}

}

namespace glengine {

void Shader::init(const char *vs_src, const char *fs_src) {
    std::vector<char const*> vs_srcs(1,vs_src);
    std::vector<char const*> fs_srcs(1,fs_src);
    init(vs_srcs, fs_srcs);
}

void Shader::init(const std::vector<char const*> &vs_srcs, const std::vector<char const*> &fs_srcs) {
    vertex_shader_id = create_shader(vs_srcs, GL_VERTEX_SHADER);
    fragment_shader_id = create_shader(fs_srcs, GL_FRAGMENT_SHADER);
    program_id = create_program();
    extract_uniforms();
    update_uniforms();
}

GLuint Shader::create_shader(const std::vector<char const*> &source, GLenum shader_type) {
    GLuint shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, source.size(), source.data(), NULL);
    glCompileShader(shader_id);
    GLint ret = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &ret);
    if (ret == GL_FALSE) {
        printf("%s: error compiling shader\n", shader_type_map[shader_type].c_str());
        print_shader_info_log(shader_id);
        printf("shader source:\n---------------\n");
        for (auto s: source) {
            printf("%s\n--------------\n",s);
        }
        return 0;
    }
    return shader_id;
}

GLuint Shader::create_program() {
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    // glBindFragDataLocation(program_id, 0, "fragment_color");
    glLinkProgram(program_id);
    GLint ret = 0;
    glGetProgramiv(program_id, GL_LINK_STATUS, &ret);
    if (ret == GL_FALSE) {
        printf("error linking shader program\n");
        print_program_info_log(program_id);
    }
    return program_id;
}

void Shader::extract_uniforms() {
    int num_uniforms = -1;
    glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &num_uniforms);
    printf(" --- Shader '%s' (%d) uniforms ---\n", name.c_str(), program_id);
    for (int i = 0; i < num_uniforms; ++i) {
        int size = -1;
        GLenum type = GL_ZERO;
        char name[512];
        glGetActiveUniform(program_id, GLuint(i), sizeof(name), NULL, &size, &type, name);
        GLint location = glGetUniformLocation(program_id, name);
        _uniforms[name] = {location, type};
        printf("%20s | %12s | %d\n", name, to_string(type), location);
    }
}

void Shader::activate() {
    glUseProgram(program_id);
}

void Shader::deactivate() {
    glUseProgram(0);
}

void Shader::update_uniforms() {
    glUseProgram(program_id);
    u_id = glGetUniformLocation(program_id, "u_id");
    u_model = glGetUniformLocation(program_id, "u_model");
    u_view = glGetUniformLocation(program_id, "u_view");
    u_projection = glGetUniformLocation(program_id, "u_projection");
    u_color = glGetUniformLocation(program_id, "u_color");
    u_light0_pos = glGetUniformLocation(program_id, "u_light0_pos");
}

void Shader::set_uniform_id(ID id) {
    glUniform1ui(u_id, id);
}

void Shader::set_uniform_model(const math::Matrix4f &m) {
    glUniformMatrix4fv(u_model, 1, GL_FALSE, m);
}

void Shader::set_uniform_view(const math::Matrix4f &v) {
    glUniformMatrix4fv(u_view, 1, GL_FALSE, v);
}

void Shader::set_uniform_projection(const math::Matrix4f &p) {
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, p);
}

void Shader::set_uniform_color(const Color &c) {
    glUniform4fv(u_color, 1, math::Vector4f(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f));
}

void Shader::set_uniform_light0_pos(const math::Vector3f &pos) {
    glUniform3fv(u_light0_pos, 1, pos);
}

bool Shader::has_uniform(const char *name) const {
    return _uniforms.count(name);
}

void Shader::set_bool(const char *name, bool value) const {
    assert(validate_set_uniform_call(name, GL_BOOL));
    glUniform1i(_uniforms.at(name).location, (int)value);
}

void Shader::set_int(const char *name, int value) const {
    assert(validate_set_uniform_call(name, GL_INT));
    glUniform1i(_uniforms.at(name).location, value);
}

void Shader::set_float(const char *name, float value) const {
    assert(validate_set_uniform_call(name, GL_FLOAT));
    glUniform1f(_uniforms.at(name).location, value);
}

void Shader::set_vec2(const char *name, const math::Vector2f &value) const {
    assert(validate_set_uniform_call(name, GL_FLOAT_VEC2));
    glUniform2fv(_uniforms.at(name).location, 1, value);
}

void Shader::set_vec3(const char *name, const math::Vector3f &value) const {
    assert(validate_set_uniform_call(name, GL_FLOAT_VEC3));
    glUniform3fv(_uniforms.at(name).location, 1, value);
}

void Shader::set_vec4(const char *name, const math::Vector4f &value) const {
    assert(validate_set_uniform_call(name, GL_FLOAT_VEC4));
    glUniform4fv(_uniforms.at(name).location, 1, value);
}

void Shader::set_mat3(const char *name, const math::Matrix3f &mat) const {
    assert(validate_set_uniform_call(name, GL_FLOAT_MAT3));
    glUniformMatrix3fv(_uniforms.at(name).location, 1, GL_FALSE, mat);
}

void Shader::set_mat4(const char *name, const math::Matrix4f &mat) const {
    assert(validate_set_uniform_call(name, GL_FLOAT_MAT4));
    glUniformMatrix4fv(_uniforms.at(name).location, 1, GL_FALSE, mat);
}

void Shader::set_sampler(const char *name, int value) const {
    assert(validate_set_uniform_call(name, GL_SAMPLER_2D));
    glUniform1i(_uniforms.at(name).location, value);
}

void Shader::print_shader_info_log(GLuint shader) {
    int max_length = 2048;
    char log[2048];
    glGetShaderInfoLog(shader, max_length, NULL, log);
    printf("shader info log for GL index %u:\n%s", shader, log);
}

void Shader::print_program_info_log(GLuint program) {
    int max_length = 2048;
    char log[2048];
    glGetProgramInfoLog(program, max_length, NULL, log);
    printf("program info log for GL index %u:\n%s", program, log);
}

bool Shader::validate_set_uniform_call(const char *name, GLenum type) const {
    if (!has_uniform(name)) {
        printf("uniform name '%s' not found in shader %d\n",name, program_id);
        return false;
    }
    const Uniform &u = _uniforms.at(name);
    if (u.type != type) {
        printf("uniform type '%s' does not match call ('%s')\n", to_string(u.type), to_string(type));
        return false;
    }
    if (u.location <0) {
        printf("invalid uniform location (%d)\n", u.location);
        return false;
    }
    return true;
}

} // namespace glengine
