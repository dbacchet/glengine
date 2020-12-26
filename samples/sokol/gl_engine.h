#pragma once

#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "imgui/imgui.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace glengine {

class GLEngine {
  public:
    virtual ~GLEngine();

    // ////// //
    // engine //
    // ////// //

    bool init(const Config &config = {});

    bool render();

    bool terminate();

    // // //
    // UI //
    // // //
    /// add a function to be called during the UI rendering
    void add_ui_function(std::function<void(void)> fun);

    // protected:
    Config _config;
    Context _context;
    Camera _camera;
    CameraManipulator _camera_manipulator;

    std::vector<std::function<void(void)>> _ui_functions;
};

} // namespace glengine
