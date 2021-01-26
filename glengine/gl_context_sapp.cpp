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

namespace {

struct InputState {
    bool left_button_pressed = false;
    bool middle_button_pressed = false;
    bool right_button_pressed = false;
    bool ctrl_key_pressed = false;
    bool shift_key_pressed = false;
    bool alt_key_pressed = false;
};

InputState g_input_state;
} // namespace

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
    // only handle specific key events if imgui is not capturing keyboard input
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        // close window
        if (e->type == SAPP_EVENTTYPE_KEY_UP && e->key_code == SAPP_KEYCODE_ESCAPE) {
            log_debug("ESC pressed: requesting quit\n");
            sapp_request_quit();
        }
        if (e->type == SAPP_EVENTTYPE_QUIT_REQUESTED) {
            log_debug("quit requested\n");
        }
        // keyboard
        if (e->type == SAPP_EVENTTYPE_KEY_DOWN || e->type == SAPP_EVENTTYPE_KEY_UP) {
            if (e->key_code == SAPP_KEYCODE_LEFT_CONTROL || e->key_code == SAPP_KEYCODE_RIGHT_CONTROL) {
                g_input_state.ctrl_key_pressed = e->type == SAPP_EVENTTYPE_KEY_DOWN;
            }
            if (e->key_code == SAPP_KEYCODE_LEFT_SHIFT || e->key_code == SAPP_KEYCODE_RIGHT_SHIFT) {
                g_input_state.shift_key_pressed = e->type == SAPP_EVENTTYPE_KEY_DOWN;
            }
            if (e->key_code == SAPP_KEYCODE_LEFT_ALT || e->key_code == SAPP_KEYCODE_RIGHT_ALT) {
                g_input_state.alt_key_pressed = e->type == SAPP_EVENTTYPE_KEY_DOWN;
            }
        }
        // statistics
        if (e->type == SAPP_EVENTTYPE_KEY_UP && e->key_code == SAPP_KEYCODE_F5) {
            _eng->_config.show_imgui_statistics = !_eng->_config.show_imgui_statistics;
        }
        // buffers and tuning
        if (e->type == SAPP_EVENTTYPE_KEY_UP && e->key_code == SAPP_KEYCODE_F6) {
            _eng->_config.show_framebuffer_texture = !_eng->_config.show_framebuffer_texture;
        }
    }
    // if imgui handled the event just return
    if (simgui_handle_event(e)) {
        return;
    }
    // mouse
    if (e->type == SAPP_EVENTTYPE_MOUSE_DOWN) {
        switch (e->mouse_button) {
        case SAPP_MOUSEBUTTON_LEFT:
            sapp_lock_mouse(true);
            g_input_state.left_button_pressed = true;
            break;
        case SAPP_MOUSEBUTTON_MIDDLE:
            g_input_state.middle_button_pressed = true;
            break;
        case SAPP_MOUSEBUTTON_RIGHT:
            g_input_state.right_button_pressed = true;
            break;
        default:
            break;
        }
    }
    if (e->type == SAPP_EVENTTYPE_MOUSE_UP) {
        switch (e->mouse_button) {
        case SAPP_MOUSEBUTTON_LEFT:
            sapp_lock_mouse(false);
            g_input_state.left_button_pressed = false;
            break;
        case SAPP_MOUSEBUTTON_MIDDLE:
            g_input_state.middle_button_pressed = false;
            break;
        case SAPP_MOUSEBUTTON_RIGHT:
            g_input_state.right_button_pressed = false;
            break;
        default:
            break;
        }
    }
    // zoom
    if (e->type == SAPP_EVENTTYPE_MOUSE_SCROLL) {
        _eng->_camera_manipulator.set_distance(_eng->_camera_manipulator.distance() * (1 - e->scroll_y / 10));
    }
    if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
        // azimuth/elevation
        if (sapp_mouse_locked() && (g_input_state.left_button_pressed && g_input_state.ctrl_key_pressed == false &&
                                    g_input_state.shift_key_pressed == false)) {
            _eng->_camera_manipulator.add_azimuth(-0.003f * e->mouse_dx);
            _eng->_camera_manipulator.set_elevation(
                math::utils::clamp<float>(_eng->_camera_manipulator.elevation() - 0.003f * e->mouse_dy, 0, M_PI));
        }
        // translate view center
        if (g_input_state.middle_button_pressed ||
            (g_input_state.left_button_pressed && g_input_state.ctrl_key_pressed == true)) {
            float scaling = 0.001f * _eng->_camera_manipulator.distance();
            float dx = scaling * e->mouse_dx;
            float dy = scaling * e->mouse_dy;
            float azimuth = _eng->_camera_manipulator.azimuth();
            math::Vector3f delta(-std::cos(azimuth) * dx - std::sin(azimuth) * dy,
                                 -std::sin(azimuth) * dx + std::cos(azimuth) * dy, 0.0f);
            _eng->_camera_manipulator.translate(delta);
        }
        // zoom view
        if (g_input_state.right_button_pressed ||
            (g_input_state.left_button_pressed && g_input_state.shift_key_pressed == true)) {
            _eng->_camera_manipulator.set_distance(_eng->_camera_manipulator.distance() * (1 - e->mouse_dy / 100.0f));
        }
    }
    // if (e->type == SAPP_EVENTTYPE_RESIZED) {
    //     create_offscreen_pass(e->framebuffer_width, e->framebuffer_height);
    // }
}
} // namespace glengine
