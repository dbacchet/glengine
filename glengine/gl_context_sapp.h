#pragma once

#include "gl_context.h"
#include "sokol_app.h"
#include <cstdint>

namespace glengine {

class ContextSapp : public Context {
  public:
    /// initialize the context
    virtual bool init(const Config &config) final;
    /// destroy the context
    virtual bool destroy() final;
    // info on window and framebuffer size
    virtual int window_width() const final;
    virtual int window_height() const final;
    virtual int framebuffer_width() const final;
    virtual int framebuffer_height() const final;

    /// handle app events
    void handle_event(const sapp_event *e);
};

} // namespace glengine
