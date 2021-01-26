#pragma once

#include <cstdint>
#include <string>

namespace glengine {


class GLEngine;

class Context {
public: 
    // generic context config parameters
    struct Config {
        uint32_t window_width = 1280;
        uint32_t window_height = 720;
        std::string window_title = "GLEngine";
        bool vsync = true;
    };
  public:
    /// initialize the context
    virtual bool init(const Config &config) = 0;
    /// destroy the context
    virtual bool destroy() = 0;
    /// register a specific engine instance
    virtual void register_engine_instance(GLEngine *eng) { _eng = eng; }
    /// called at the beginning of a frame in the main loop.
    /// This function is usually a good place to update per-frame info, like the imgui input
    virtual void begin_frame(){};
    /// called at the end of a frame in the main loop.
    /// \return flag indicating that the context is still valid (window not closing for example)
    virtual bool end_frame() { return true; }
    // info on window and framebuffer size
    virtual int window_width() const = 0;
    virtual int window_height() const = 0;
    virtual int framebuffer_width() const = 0;
    virtual int framebuffer_height() const = 0;

    GLEngine *_eng = nullptr;
};

} // namespace glengine
