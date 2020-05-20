#include "gl_shader.h"

#include <unordered_map>

namespace {

std::unordered_map<GLenum, std::string> shader_type_map = {
    {GL_FRAGMENT_SHADER, "FRAGMENT_SHADER"},
    {GL_VERTEX_SHADER, "VERTEX_SHADER"},
};

}

namespace glengine {

void Shader::init(const char *vs_src, const char *fs_src) {
    vertex_shader_id = create_shader(vs_src, GL_VERTEX_SHADER);
    fragment_shader_id = create_shader(fs_src, GL_FRAGMENT_SHADER);
    program_id = create_program();
    update_uniforms();
}

GLuint Shader::create_shader(const char *source, GLenum shader_type) {
    GLuint shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &source, NULL);
    glCompileShader(shader_id);
    GLint ret = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &ret);
    if (ret == GL_FALSE) {
        printf("%s: error compiling shader\n", shader_type_map[shader_type].c_str());
        print_shader_info_log(shader_id);
        return 0;
    }
    return shader_id;
}

GLuint Shader::create_program() {
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glBindFragDataLocation(program_id, 0, "fragment_color");
    glLinkProgram(program_id);
    GLint ret = 0;
    glGetProgramiv(program_id, GL_LINK_STATUS, &ret);
    if (ret == GL_FALSE) {
        printf("error linking shader program\n");
        print_program_info_log(program_id);
    }
    return program_id;
}

void Shader::activate() {
    glUseProgram(program_id);
}

void Shader::deactivate() {
    glUseProgram(0);
}

void Shader::update_uniforms() {
    glUseProgram(program_id);
    u_model = glGetUniformLocation(program_id, "u_model");
    u_view = glGetUniformLocation(program_id, "u_view");
    u_projection = glGetUniformLocation(program_id, "u_projection");
    u_color = glGetUniformLocation(program_id, "u_color");
    u_light0_pos = glGetUniformLocation(program_id, "u_light0_pos");
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
    set_uniform_color(math::Vector4f(c.r/255.0f,c.g/255.0f,c.b/255.0f,c.a/255.0f));
}

void Shader::set_uniform_color(const math::Vector4f &c) {
    glUniform4fv(u_color, 1, c);
}

void Shader::set_uniform_light0_pos(const math::Vector3f &pos) {
    glUniform3fv(u_light0_pos, 1, pos);
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

} // namespace glengine
