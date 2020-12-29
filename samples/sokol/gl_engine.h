#pragma once

#include "gl_context.h"
#include "gl_camera.h"
#include "gl_camera_manipulator.h"
#include "gl_object.h"

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

    // ///////////// //
    // objects //
    // ///////////// //
    /// create a new (empty) object
    Object *create_object(Object *parent=nullptr, ID id=NULL_ID);
    /// create a new object and add the given renderable
    Object *create_object(const Renderable &renderable, Object *parent=nullptr, ID id=NULL_ID);
    /// create a new object, given an array of renderables
    Object *create_object(const std::vector<Renderable> &renderables, Object *parent=nullptr, ID id=NULL_ID);

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

    Object *_root = nullptr;

    std::vector<std::function<void(void)>> _ui_functions;
};

} // namespace glengine
