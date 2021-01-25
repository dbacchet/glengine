#pragma once

#include "gl_context.h"
#include <cstdint>

namespace glengine {

class ContextGLFW : public Context {
  public:
    /// initialize the context
    virtual bool init(const Config &config) final;
    /// destroy the context
    virtual bool destroy() final;
    /// register engine instance 
    virtual void register_engine_instance(GLEngine *eng);
    /// call at the beginning of a frame in the main loop.
    /// This function is usually a good place to update per-frame info, like the imgui input
    virtual void begin_frame() final;
    /// call at the end of a frame in the main loop.
    /// \return flag indicating that the context is still valid (window not closing for example)
    virtual bool end_frame() final;
    // info on window and framebuffer size
    virtual int window_width() const final;
    virtual int window_height() const final;
    virtual int framebuffer_width() const final;
    virtual int framebuffer_height() const final;
};

} // namespace glengine
