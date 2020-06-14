#pragma once

#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "gl_renderobject.h"
#include "gl_resource_manager.h"
#include "imgui/imgui.h"

#include <cstdint>
#include <unordered_map>
#include <functional>
#include <limits>

namespace glengine {

class Mesh;
class Shader;

class GLEngine {
  public:
    virtual ~GLEngine();

    // ////// //
    // engine //
    // ////// //

    bool init(const Config &config = {});

    bool render();

    bool terminate();

    /// get resource manager
    ResourceManager& resource_manager() {
        return _resource_manager;
    }

    // ///////////// //
    // renderobjects //
    // ///////////// //
    /// create a new (uninitialized) renderobject
    RenderObject *create_renderobject(RenderObject *parent=nullptr, ID id=NULL_ID);
    /// create a new renderobject, given a mesh and a shader
    RenderObject *create_renderobject(Mesh *mesh, Shader *shader, RenderObject *parent=nullptr, ID id=NULL_ID);
    /// create a new renderobject, given an array of meshes and a shader
    RenderObject *create_renderobject(const std::vector<Mesh*> &meshes, Shader *shader, RenderObject *parent=nullptr, ID id=NULL_ID);

    // // //
    // UI //
    // // //
    /// add a function to be called during the UI rendering
    void add_ui_function(std::function<void(void)> fun);

    // ///////// //
    // selection //
    // ///////// //
    /// get the current cursor coordinates
    math::Vector2i cursor_pos() const;
    /// get the id of the object at the given (screen) coordinates.
    /// \return object id or NULL_ID if none
    ID object_at_screen_coord(const math::Vector2i &cursor_pos) const;

    void save_screenshot(const char *filename);

    // protected:
    Config _config;
    Context _context;
    Camera _camera;
    CameraManipulator _camera_manipulator;
    ResourceManager _resource_manager;

    GLuint _g_buffer = 0;              // framebuffer id
    GLuint _ssao_framebuffer = 0;      // SSAO framebuffer id
    GLuint _ssao_blur_framebuffer = 0;      // blur framebuffer id
    GLuint _gb_position = INVALID_BUFFER; // framebuffer position attachment handle
    GLuint _gb_normal = INVALID_BUFFER; // framebuffer normal attachment handle
    GLuint _gb_albedo = INVALID_BUFFER; // framebuffer albedo attachment handle
    GLuint _gb_color = INVALID_BUFFER; // framebuffer color attachment handle
    GLuint _gb_id = INVALID_BUFFER;    // framebuffer object_id attachment handle
    GLuint _gb_depth = INVALID_BUFFER; // framebuffer depth+stencil attachment handle
    GLuint _ssao_color_texture = INVALID_BUFFER; // ssao framebuffer color attachment handle
    GLuint _ssao_noise_texture = INVALID_BUFFER; // texture used to randomly rotate the ssao kernel
    GLuint _ssao_blur_color_texture = INVALID_BUFFER; // ssao blur framebuffer color attachment handle
    static constexpr uint32_t num_ssao_kernel_samples = 64;
    math::Vector3f ssaoKernel[num_ssao_kernel_samples] = {};
    Mesh *_ss_quad = nullptr;
    std::vector<ID> _id_buffer; // buffer containing the id of the object in every pixel

    RenderObject *_root = nullptr;

    std::vector<std::function<void(void)>> _ui_functions;

    void resize_buffers();
};

} // namespace glengine
