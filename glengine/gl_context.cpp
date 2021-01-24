#include "gl_context.h"

#include "math/vmath.h"
#include "sokol_app.h"

#include <cstdint>

namespace glengine {

// initialize the opengl context
bool init_context(const Config &config, const char *title, void *user_pointer) {
    return true;
}
// destroy the given context
void destroy_context() {}
// function called at the beginning of each frame
// This is usually a good place to update the inputs to imgui, etc
void begin_frame() {}
// function called at the end of each frame
// This is typically a good place to trigger the swap chain and the rendering
void end_frame() {}
// return if the window is still valid or being closed
bool window_should_close() {return false;}
// return the size of the window
math::Vector2i window_size() {
    return math::Vector2i(sapp_width(), sapp_height());
}
// return the size of the display framebuffer (can be different from window in highdpi displays)
math::Vector2i framebuffer_size() {
    return math::Vector2i(sapp_width()*sapp_dpi_scale(), sapp_height()*sapp_dpi_scale());
}
} // namespace glengine
