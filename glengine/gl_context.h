#pragma once

#include "math/vmath.h"
#include <cstdint>


namespace glengine {

// generic context config parameters
struct Config {
    uint32_t window_width = 1280;
    uint32_t window_height = 720;
    bool vsync = true;
    // debug flags
    bool show_framebuffer_texture = false;
    bool show_imgui_statistics = false;
    uint16_t msaa_samples = 4;
    bool use_mrt = false;
};

/// initialize the opengl context
bool init_context(const Config &config = {}, const char *title = "sample", void *user_pointer=nullptr);
/// destroy the given context
void destroy_context();
/// function called at the beginning of each frame
/// This is usually a good place to update the inputs to imgui, etc
void begin_frame();
/// function called at the end of each frame
/// This is typically a good place to trigger the swap chain and the rendering
void end_frame();
/// return if the window is still valid or being closed
bool window_should_close();
/// return the size of the window
math::Vector2i window_size();
/// return the size of the display framebuffer (can be different from window in highdpi displays)
math::Vector2i framebuffer_size();

} // namespace glengine
