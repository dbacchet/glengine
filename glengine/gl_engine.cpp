#include "gl_engine.h"
#include "gl_renderobject.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "math/vmath.h"
#include "math/math_utils.h"

#include "stb/stb_image_write.h"

#include <random>

GLuint myFBO = 0;
GLuint myTex = 0;

const char *myshader_vs =
    R"(#version 330
// vertex attributes
layout (location=0) in vec3 v_position;
layout (location=1) in vec4 v_color;
layout (location=2) in vec3 v_normal;
layout (location=3) in vec2 v_texcoord0;
// outputs
out vec2 texcoord;

void main()
{
    texcoord = v_texcoord0;
    gl_Position = vec4(v_position.x, v_position.y, 0.0, 1.0); 
})";

const char *myshader_fs =
    R"(#version 330
// uniforms
uniform sampler2D g_position_texture;
uniform sampler2D g_normal_texture;
uniform sampler2D noise_texture;
uniform mat4 u_projection;
uniform vec3 samples[64];
// inputs
in vec2 texcoord;
// outputs
out vec4 fragment_color;

int kernelSize = 64;
float radius = 1.5;
float bias = 0.025;
const vec2 noise_scale = vec2(1280.0/4.0, 720.0/4.0); 

void main()
{
    // get inputs
    vec3 frag_pos = texture(g_position_texture, texcoord).xyz;
    vec3 normal = normalize(texture(g_normal_texture, texcoord).rgb);
    vec3 random_vec = normalize(texture(noise_texture, texcoord * noise_scale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // from tangent to view-space
        sample = frag_pos + sample * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = u_projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sample_depth = texture(g_position_texture, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);


    if (false) {
        // if (texcoord.x<0.5) {
        //     if (texcoord.y<0.5) {
        //         // bottom left
        //         fragment_color = vec4(frag_pos, 1.0);
        //     } else {
        //         // top left
        //         fragment_color = vec4(occlusion, occlusion, occlusion, 1.0);
        //     }
        // } else {
        //     if (texcoord.y<0.5) {
        //         // bottom right
        //         fragment_color = vec4(random_vec, 1.0);
        //     } else {
        //         // top right
        //         fragment_color = vec4(normal, 1.0);
        //     }
        // }
    } else {
        fragment_color = vec4(occlusion, occlusion, occlusion, 1.0);
        // fragment_color = occlusion;
    }
})";

glengine::Shader *myshader = new glengine::Shader();


namespace {

int saveScreenshot(const char *filename)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    int x = viewport[0];
    int y = viewport[1];
    int width = viewport[2];
    int height = viewport[3];

    char *data = (char*) malloc((size_t) (width * height * 3)); // 3 components (R, G, B)

    if (!data)
        return 0;

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_flip_vertically_on_write(true);
    int saved = stbi_write_png(filename, width, height, 3, data, 0);

    free(data);

    return saved;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
        ctx.input_state.ctrl_key_pressed = action == GLFW_PRESS;
    }
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
        ctx.input_state.shift_key_pressed = action == GLFW_PRESS;
    }
    if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT) {
        ctx.input_state.alt_key_pressed = action == GLFW_PRESS;
    }
}

void scroll_callback(GLFWwindow *window, double xoffs, double yoffs) {
    if (ImGui::GetIO().WantCaptureMouse) {
        // do nothing if the mouse is on top a UI element
        return;
    }
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    app._camera_manipulator.set_distance(app._camera_manipulator.distance() * (1 - yoffs / 10));
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    auto &cm = app._camera_manipulator;
    math::Vector2i cursor_pos(int(xpos + 0.5), int(ypos + 0.5));
    math::Vector2i cursor_delta(0, 0);
    if (ctx.input_state.previous_cursor_pos != math::Vector2i(-1, -1)) {
        cursor_delta = cursor_pos - ctx.input_state.previous_cursor_pos;
    }
    // rotate view
    if (ctx.input_state.left_button_pressed && ctx.input_state.ctrl_key_pressed == false) {
        cm.add_azimuth(-0.003f * cursor_delta.x);
        // cm.add_elevation(-0.003f * cursor_delta.y);
        cm.set_elevation(math::utils::clamp<float>(cm.elevation() - 0.003f * cursor_delta.y, 0, M_PI));
    }
    // translate view center
    if (ctx.input_state.middle_button_pressed ||
        (ctx.input_state.left_button_pressed && ctx.input_state.ctrl_key_pressed == true)) {
        float scaling = 0.001f * cm.distance();
        float dx = scaling * cursor_delta.x;
        float dy = scaling * cursor_delta.y;
        float azimuth = cm.azimuth();
        math::Vector3f delta(-std::cos(azimuth) * dx - std::sin(azimuth) * dy,
                             -std::sin(azimuth) * dx + std::cos(azimuth) * dy, 0.0f);
        cm.translate(delta);
    }
    // zoom view
    if (ctx.input_state.right_button_pressed) {
        cm.set_distance(cm.distance() * (1 - cursor_delta.y / 100.0f));
    }
    ctx.input_state.previous_cursor_pos = cursor_pos;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse) {
        // do nothing if the mouse is on top a UI element
        return;
    }
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx.input_state.left_button_pressed = action == GLFW_PRESS;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        ctx.input_state.middle_button_pressed = action == GLFW_PRESS;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        ctx.input_state.right_button_pressed = action == GLFW_PRESS;
    }
}

void window_size_callback(GLFWwindow *window, int width, int height) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    ctx.window_state.window_size = {width, height};
    printf("win size: %d %d\n", width, height);
}

void framebuffer_size_callback(GLFWwindow *window, int fb_width, int fb_height) {
    auto &app = *(glengine::GLEngine *)glfwGetWindowUserPointer(window);
    auto &ctx = app._context;
    ctx.window_state.framebuffer_size = {fb_width, fb_height};
    printf("fb size: %d %d\n", fb_width, fb_height);
    app.resize_buffers();
}

} // namespace

namespace glengine {

GLEngine::~GLEngine() {}

bool GLEngine::init(const Config &config) {
    _config = config;
    _context = glengine::init_context(config, "GLEngine sample app", (void *)this,
                                      {
                                          scroll_callback,          // scroll_fun_callback
                                          mouse_button_callback,    // mousebutton_fun_callback
                                          key_callback,             // key_fun_callback
                                          cursor_position_callback, // cursorpos_fun_callback
                                          nullptr,                  // cursorenterexit_fun_callback
                                          nullptr,                  // char_fun_callback
                                          window_size_callback,
                                          framebuffer_size_callback //
                                      });

    _camera.set_perspective(0.1, 1000.0, math::utils::deg2rad(45.0f));
    _camera.set_transform(math::create_lookat<float>({-10.0f, -1.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));

    // initialize the resource manager (creates default resources)
    _resource_manager.init();
    // create root of the scene
    _root = new RenderObject();

    // ///////////////////// //
    // framebuffer: g-buffer //
    // ///////////////////// //
    glGenFramebuffers(1, &_g_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _g_buffer);
    printf("generated gbuffer FB: id=%d",_ssao_framebuffer);
    // color buffer
    glGenTextures(1, &_gb_color);
    glBindTexture(GL_TEXTURE_2D, _gb_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gb_color, 0);
    // object id buffer
    glGenTextures(1, &_gb_id);
    glBindTexture(GL_TEXTURE_2D, _gb_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _gb_id, 0);
    // deferrend rendering - position buffer
    glGenTextures(1, &_gb_position);
    glBindTexture(GL_TEXTURE_2D, _gb_position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _gb_position, 0);
    // deferrend rendering - normal buffer
    glGenTextures(1, &_gb_normal);
    glBindTexture(GL_TEXTURE_2D, _gb_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, _gb_normal, 0);
    // deferrend rendering - albedo + specular buffer
    glGenTextures(1, &_gb_albedo);
    glBindTexture(GL_TEXTURE_2D, _gb_albedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, _gb_albedo, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(5, attachments);
    // create and attach depth buffer (renderbuffer)
    glGenRenderbuffers(1, &_gb_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, _gb_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _gb_depth); // now actually attach it
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: framebuffer not complete!\n");
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ///////////////// //
    // framebuffer: tEST //
    // ///////////////// //
    glGenFramebuffers(1, &myFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, myFBO);
    // ssao color buffer
    glGenTextures(1, &myTex);
    glBindTexture(GL_TEXTURE_2D, myTex);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, myTex, 0);
    glObjectLabel(GL_TEXTURE, myTex, -1, "mytex");
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: framebuffer not complete!\n");
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    myshader->init(myshader_vs, myshader_fs);

    // ///////////////// //
    // framebuffer: SSAO //
    // ///////////////// //
    glGenFramebuffers(1, &_ssao_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _ssao_framebuffer);
    // ssao color buffer
    glGenTextures(1, &_ssao_color_texture);
    glBindTexture(GL_TEXTURE_2D, _ssao_color_texture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RED, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ssao_color_texture, 0);
    glObjectLabel(GL_TEXTURE, _ssao_color_texture, -1, "ssao color1");
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: framebuffer not complete!\n");
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // // ///////////////// //
    // // framebuffer: blur //
    // // ///////////////// //
    // glGenFramebuffers(1, &_ssao_blur_framebuffer);
    // glBindFramebuffer(GL_FRAMEBUFFER, _ssao_blur_framebuffer);
    // // ssao color buffer
    // glGenTextures(1, &_ssao_blur_color_texture);
    // glBindTexture(GL_TEXTURE_2D, _ssao_blur_color_texture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _context.window_state.framebuffer_size.x, _context.window_state.framebuffer_size.y, 0, GL_RED, GL_FLOAT, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ssao_blur_color_texture, 0);
    // // finally check if framebuffer is complete
    // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    //     printf("Error: framebuffer not complete!\n");
    //     return false;
    // }
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
   
    // ssao samples
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::mt19937 gen(12345678);
    for (unsigned int i = 0; i < num_ssao_kernel_samples; ++i) {
        math::Vector3f sample(randomFloats(gen) * 2.0 - 1.0, randomFloats(gen) * 2.0 - 1.0, randomFloats(gen));
        math::normalize(sample);
        sample *= randomFloats(gen);
        float scale = float(i) / num_ssao_kernel_samples;
        // scale samples s.t. they're more aligned to center of kernel
        scale = math::utils::lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel[i] = sample;
    }
    // ssao noise texture
    std::vector<math::Vector3d> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        math::Vector3d noise(randomFloats(gen) * 2.0 - 1.0, randomFloats(gen) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    glGenTextures(1, &_ssao_noise_texture);
    glBindTexture(GL_TEXTURE_2D, _ssao_noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // resize the cpu buffer for the object ids
    _id_buffer.resize(_context.window_state.framebuffer_size.x*_context.window_state.framebuffer_size.y, NULL_ID);

    // create the screen quad mesh
    _ss_quad = _resource_manager.create_quad_mesh();

    float srgb_gamma = 2.2;
    glClearColor(pow(0.1f,srgb_gamma), pow(0.1f,srgb_gamma), pow(0.1f,srgb_gamma), 1.0f);
    glEnable(GL_DEPTH_TEST);

    return true;
}

bool GLEngine::render() {

    const auto &fbsize = _context.window_state.framebuffer_size;

    _camera_manipulator.update(_camera);
    _camera.update(fbsize.x, fbsize.y);

    glViewport(0, 0, fbsize.x, fbsize.y);

    // bind to framebuffer and draw scene as we normally would to color texture 
    glBindFramebuffer(GL_FRAMEBUFFER, _g_buffer);

    glViewport(0, 0, fbsize.x, fbsize.y);
    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // "clear" the id buffer setting NULL_ID as clear value
    glClearBufferuiv(GL_COLOR, 1, &NULL_ID);

    if (_root) {
        _root->draw(_camera);
    }

    // calculate ssao
    glBindFramebuffer(GL_FRAMEBUFFER, _ssao_framebuffer);
    glViewport(0, 0, fbsize.x, fbsize.y);
    glClear(GL_COLOR_BUFFER_BIT);
    Shader *ssao_shader = _resource_manager.get_stock_shader(StockShader::Ssao);
    ssao_shader->activate();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _gb_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _gb_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _ssao_noise_texture);
    glUniform1i(glGetUniformLocation(ssao_shader->program_id, "g_position_texture"), 0); 
    glUniform1i(glGetUniformLocation(ssao_shader->program_id, "g_normal_texture"), 1); 
    glUniform1i(glGetUniformLocation(ssao_shader->program_id, "noise_texture"), 2); 
    GLint samples_array_base = glGetUniformLocation(myshader->program_id, "samples[0]");
    for (uint32_t i=0; i<num_ssao_kernel_samples; i++) { // all the elements in the array have contiguous unifor ids
        glUniform3fv(samples_array_base+i, 1, ssaoKernel[i]);
    }
    ssao_shader->set_uniform_projection(_camera.projection());
    _ss_quad->draw(*ssao_shader);

    // test stage
    glBindFramebuffer(GL_FRAMEBUFFER, myFBO);
    glViewport(0, 0, fbsize.x, fbsize.y);
    myshader->activate();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _gb_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _gb_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _ssao_noise_texture);
    glUniform1i(glGetUniformLocation(myshader->program_id, "g_position_texture"), 0); 
    glUniform1i(glGetUniformLocation(myshader->program_id, "g_normal_texture"), 1); 
    glUniform1i(glGetUniformLocation(myshader->program_id, "noise_texture"), 2); 
    samples_array_base = glGetUniformLocation(myshader->program_id, "samples[0]");
    for (uint32_t i=0; i<num_ssao_kernel_samples; i++) {
        glUniform3fv(samples_array_base+i, 1, ssaoKernel[i]);
    }
    myshader->set_uniform_projection(_camera.projection());
    _ss_quad->draw(*myshader);

    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    glEnable(GL_FRAMEBUFFER_SRGB); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

    static bool use_deferred = false;
    glViewport(0, 0, fbsize.x, fbsize.y);
    Shader *quad_shader = use_deferred ? _resource_manager.get_stock_shader(StockShader::QuadDeferred) : _resource_manager.get_stock_shader(StockShader::Quad);
    quad_shader->activate();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _gb_color);	// use the color attachment texture as the texture of the quad plane
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _gb_position);	// use the color attachment texture as the texture of the quad plane
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _gb_normal);	// use the color attachment texture as the texture of the quad plane
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _gb_albedo);	// use the color attachment texture as the texture of the quad plane
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, _ssao_color_texture);	// use the color attachment texture as the texture of the quad plane
    glUniform1i(glGetUniformLocation(quad_shader->program_id, "screen_texture"), 0); 
    glUniform1i(glGetUniformLocation(quad_shader->program_id, "g_position_texture"), 1); 
    glUniform1i(glGetUniformLocation(quad_shader->program_id, "g_normal_texture"), 2); 
    glUniform1i(glGetUniformLocation(quad_shader->program_id, "g_albedospec_texture"), 3); 
    glUniform1i(glGetUniformLocation(quad_shader->program_id, "ssao_texture"), 4); 
    quad_shader->set_uniform_view(_camera.inverse_transform());
    _ss_quad->draw(*quad_shader);
    glDisable(GL_FRAMEBUFFER_SRGB); 

    // copy the id texture in cpu memory
    glBindTexture(GL_TEXTURE_2D, _gb_id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, _id_buffer.data());

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (_config.show_imgui_statistics) {
        ImGui::ShowMetricsWindow();
    }
    if (_config.show_framebuffer_texture) {
        int img_width = 250;
        int img_height = img_width*(float)fbsize.y/fbsize.x;
        ImGui::Begin("fb images");
        ImGui::Checkbox("Deferred Rendering", &use_deferred);
        ImGui::Image((void*)(intptr_t)_gb_color, ImVec2(img_width,img_height),ImVec2(0,1),ImVec2(1,0));
        // ImGui::Image((void*)(intptr_t)_gb_position, ImVec2(img_width,img_height),ImVec2(0,1),ImVec2(1,0));
        // ImGui::Image((void*)(intptr_t)_gb_normal, ImVec2(img_width,img_height),ImVec2(0,1),ImVec2(1,0));
        // ImGui::Image((void*)(intptr_t)_gb_albedo, ImVec2(img_width,img_height),ImVec2(0,1),ImVec2(1,0));
        ImGui::Image((void*)(intptr_t)_ssao_color_texture, ImVec2(img_width,img_height),ImVec2(0,1),ImVec2(1,0));
        ImGui::Image((void*)(intptr_t)myTex, ImVec2(img_width,img_height),ImVec2(0,1),ImVec2(1,0));
        ImGui::End();
    }

    for (auto& fun: _ui_functions) {
        fun();
    }

    // render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(_context.window);
    glfwPollEvents();
    return !glfwWindowShouldClose(_context.window);
}

bool GLEngine::terminate() {
    // deallocate all resources
    delete _root;
    _root = nullptr;
    glengine::destroy_context(_context);
    return true;
}

RenderObject *GLEngine::create_renderobject(RenderObject *parent, ID id) {
    RenderObject *ro = new RenderObject(parent ? parent : _root, id);
    return ro;
}

RenderObject *GLEngine::create_renderobject(Mesh *mesh, Shader *shader, RenderObject *parent, ID id) {
    RenderObject *ro = create_renderobject(parent, id);
    ro->init(mesh, shader);
    return ro;
}

RenderObject *GLEngine::create_renderobject(const std::vector<Mesh*> &meshes, Shader *shader, RenderObject *parent, ID id) {
    RenderObject *ro = create_renderobject(parent, id);
    ro->init(meshes, shader);
    return ro;
}

void GLEngine::add_ui_function(std::function<void(void)> fun) {
    _ui_functions.push_back(fun);
}

math::Vector2i GLEngine::cursor_pos() const {
    return _context.input_state.previous_cursor_pos;
}

ID GLEngine::object_at_screen_coord(const math::Vector2i &cursor_pos) const {
    // cursor is in window coord, so we need to convert to framebuffer
    float scale_x = float(_context.window_state.framebuffer_size.x)/_context.window_state.window_size.x;
    float scale_y = float(_context.window_state.framebuffer_size.y)/_context.window_state.window_size.y;
    int32_t px = int32_t(scale_x * cursor_pos.x + 0.5f);
    int32_t py = int32_t(scale_y * (_context.window_state.window_size.y -1 - cursor_pos.y) + 0.5f);
    int32_t idx = py*_context.window_state.framebuffer_size.x+px;
    if (idx<0 || idx>=(int32_t)_id_buffer.size()) {
        return NULL_ID;
    }
    return _id_buffer[idx];
}

void GLEngine::save_screenshot(const char *filename) {
    saveScreenshot(filename);
}

void GLEngine::resize_buffers() {
    // const auto &win_size = _context.window_state.window_size;
    const auto &fb_size = _context.window_state.framebuffer_size;
    // align opengl buffers to the size of the framebuffer
    if (_gb_color!=INVALID_BUFFER) {
        glBindTexture(GL_TEXTURE_2D, _gb_color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb_size.x, fb_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    if (_gb_id!=INVALID_BUFFER) {
        glBindTexture(GL_TEXTURE_2D, _gb_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, fb_size.x, fb_size.y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    }
    if (_gb_position!=INVALID_BUFFER) {
        glBindTexture(GL_TEXTURE_2D, _gb_position);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fb_size.x, fb_size.y, 0, GL_RGBA, GL_FLOAT, NULL);
    }
    if (_gb_normal!=INVALID_BUFFER) {
        glBindTexture(GL_TEXTURE_2D, _gb_normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fb_size.x, fb_size.y, 0, GL_RGBA, GL_FLOAT, NULL);
    }
    if (_gb_albedo!=INVALID_BUFFER) {
        glBindTexture(GL_TEXTURE_2D, _gb_albedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb_size.x, fb_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    if (_gb_depth!=INVALID_BUFFER) {
        glBindRenderbuffer(GL_RENDERBUFFER, _gb_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fb_size.x, fb_size.y);
    }
    _id_buffer.resize(fb_size.x*fb_size.y, NULL_ID);
}

} // namespace glengine
