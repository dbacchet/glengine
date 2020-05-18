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
