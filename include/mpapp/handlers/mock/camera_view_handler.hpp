// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/CameraView.md
//
// `camera_view_handler<platform::mock>` — records property mappers and
// command invocations for the `basic_camera_view` primitive.
// Subscribes to every Observable and lifecycle signal so tests can
// assert the exact sequence and arguments the framework would pass to
// a real native handler.

#ifndef MPAPP_HANDLERS_MOCK_CAMERA_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_CAMERA_VIEW_HANDLER_HPP

#include <string>

#include "../../internal/basic_camera_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class camera_view_handler<platform::mock>
    : public mock_handler_base {
public:
    camera_view_handler()  = default;
    ~camera_view_handler() = default;

    camera_view_handler(const camera_view_handler&)            = delete;
    camera_view_handler& operator=(const camera_view_handler&) = delete;
    camera_view_handler(camera_view_handler&&)                 = delete;
    camera_view_handler& operator=(camera_view_handler&&)      = delete;

    // ----- Property mappers -------------------------------------------------

    void map_flash(basic_camera_view& cv) {
        record_change("flash", cv.flash.get());
        cv.flash.changed.subscribe(flash_slot_, flash_cb_);
    }

    void map_position(basic_camera_view& cv) {
        record_change("position", cv.position.get());
        cv.position.changed.subscribe(position_slot_, position_cb_);
    }

    void map_is_available(basic_camera_view& cv) {
        record_change("is_available", cv.is_available.get());
        cv.is_available.changed.subscribe(available_slot_, available_cb_);
    }

    void map_is_torch_on(basic_camera_view& cv) {
        record_change("is_torch_on", cv.is_torch_on.get());
        cv.is_torch_on.changed.subscribe(torch_slot_, torch_cb_);
    }

    void map_zoom_factor(basic_camera_view& cv) {
        record_change("zoom_factor", cv.zoom_factor.get());
        cv.zoom_factor.changed.subscribe(zoom_slot_, zoom_cb_);
    }

    // ----- Signal mappers ---------------------------------------------------

    // Subscribe so preview lifecycle transitions appear in the call log.
    void map_preview_started(basic_camera_view& cv) {
        cv.preview_started.subscribe(preview_started_slot_,
                                     preview_started_cb_);
    }

    void map_preview_stopped(basic_camera_view& cv) {
        cv.preview_stopped.subscribe(preview_stopped_slot_,
                                     preview_stopped_cb_);
    }

    // Subscribe to capture outcome signals (media_captured / camera_error).
    void map_media_captured(basic_camera_view& cv) {
        cv.media_captured.subscribe(media_captured_slot_, media_captured_cb_);
    }

    void map_camera_error(basic_camera_view& cv) {
        cv.camera_error.subscribe(camera_error_slot_, camera_error_cb_);
    }

private:
    using self_t = camera_view_handler<platform::mock>;

    struct flash_recorder {
        self_t* self = nullptr;
        void operator()(camera_flash v) const {
            self->record_change("flash", v);
        }
    };

    struct position_recorder {
        self_t* self = nullptr;
        void operator()(camera_position v) const {
            self->record_change("position", v);
        }
    };

    struct bool_recorder {
        self_t*     self = nullptr;
        const char* prop = "";
        void operator()(bool v) const { self->record_change(prop, v); }
    };

    struct double_recorder {
        self_t*     self = nullptr;
        const char* prop = "";
        void operator()(double v) const { self->record_change(prop, v); }
    };

    struct void_event_recorder {
        self_t*     self  = nullptr;
        const char* event = "";
        void operator()() const { self->record_event(event); }
    };

    struct string_event_recorder {
        self_t*     self  = nullptr;
        const char* event = "";
        void operator()(const std::string& /*v*/) const {
            self->record_event(event);
        }
    };

    flash_recorder          flash_cb_{this};
    position_recorder       position_cb_{this};
    bool_recorder           available_cb_{this, "is_available"};
    bool_recorder           torch_cb_{this, "is_torch_on"};
    double_recorder         zoom_cb_{this, "zoom_factor"};
    void_event_recorder     preview_started_cb_{this, "start_preview"};
    void_event_recorder     preview_stopped_cb_{this, "stop_preview"};
    string_event_recorder   media_captured_cb_{this, "media_captured"};
    string_event_recorder   camera_error_cb_{this, "camera_error"};

    signal_slot<const camera_flash&>    flash_slot_{};
    signal_slot<const camera_position&> position_slot_{};
    signal_slot<const bool&>            available_slot_{};
    signal_slot<const bool&>            torch_slot_{};
    signal_slot<const double&>          zoom_slot_{};
    signal_slot<>                       preview_started_slot_{};
    signal_slot<>                       preview_stopped_slot_{};
    signal_slot<std::string>            media_captured_slot_{};
    signal_slot<std::string>            camera_error_slot_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_CAMERA_VIEW_HANDLER_HPP
