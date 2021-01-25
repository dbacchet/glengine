#include "gl_context_sapp.h"
#include "gl_logger.h"
#include "gl_engine.h"

#include "math/math_utils.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "imgui/imgui.h"
#include "sokol_imgui.h"
#include "sokol_glue.h"

#include <cstdint>

namespace glengine {

// initialize the context
bool ContextSapp::init(const Config &config) {
    sg_setup((sg_desc){.context = sapp_sgcontext()});
    return true;
}
// destroy the context
bool ContextSapp::destroy() {
    sg_shutdown();
    return true;
}
// info on window and framebuffer size
int ContextSapp::window_width() const {
    return sapp_width();
}
int ContextSapp::window_height() const {
    return sapp_height();
}
int ContextSapp::framebuffer_width() const {
    return int(sapp_width() * sapp_dpi_scale());
}
int ContextSapp::framebuffer_height() const {
    return int(sapp_height() * sapp_dpi_scale());
}

void ContextSapp::handle_event(const sapp_event *e) {
    // close window
    if (e->type == SAPP_EVENTTYPE_KEY_UP && e->key_code == SAPP_KEYCODE_ESCAPE) {
        log_debug("ESC pressed: requesting quit\n");
        if (!ImGui::GetIO().WantCaptureKeyboard) {
            sapp_request_quit();
        }
    }
    if (e->type == SAPP_EVENTTYPE_QUIT_REQUESTED) {
        log_debug("quit requested\n");
    }
    // if imgui handled the event just return
    if (simgui_handle_event(e)) {
        return;
    }
    // if (e->type == SAPP_EVENTTYPE_RESIZED) {
    //     create_offscreen_pass(e->framebuffer_width, e->framebuffer_height);
    // }
    // zoom
    if (e->type == SAPP_EVENTTYPE_MOUSE_SCROLL) {
        _eng->_camera_manipulator.set_distance(_eng->_camera_manipulator.distance() * (1 - e->scroll_y / 10));
    }
    // azimuth/elevation
    if (e->type == SAPP_EVENTTYPE_MOUSE_DOWN) {
        // printf("button down %d\n", e->mouse_button);
        if (e->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            sapp_lock_mouse(true);
        }
    }
    if (e->type == SAPP_EVENTTYPE_MOUSE_UP) {
        // printf("button up %d\n", e->mouse_button);
        if (e->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            sapp_lock_mouse(false);
        }
    }
    if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
        if (sapp_mouse_locked()) {
            _eng->_camera_manipulator.add_azimuth(-0.003f * e->mouse_dx);
            _eng->_camera_manipulator.set_elevation(
                math::utils::clamp<float>(_eng->_camera_manipulator.elevation() - 0.003f * e->mouse_dy, 0, M_PI));
        }
    }
}
} // namespace glengine
