// SPDX-License-Identifier: Apache-2.0
// Android graphics_view handler — wraps a plain android.view.View
// sized via setMinimumWidth/Height. User-facing Canvas drawing API
// gated on ADR-0015.

#ifndef MPAPP_HANDLERS_ANDROID_GRAPHICS_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_GRAPHICS_VIEW_HANDLER_HPP

#include "../../graphics_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class graphics_view_handler<platform::android> {
public:
    graphics_view_handler();
    ~graphics_view_handler();

    graphics_view_handler(const graphics_view_handler&)            = delete;
    graphics_view_handler& operator=(const graphics_view_handler&) = delete;
    graphics_view_handler(graphics_view_handler&&)                 = delete;
    graphics_view_handler& operator=(graphics_view_handler&&)      = delete;

    void map_size(graphics_view& gv);
    void map_draw_count(graphics_view& gv);

    jobject native() const noexcept { return native_; }

private:
    void apply_width(int w);
    void apply_height(int h);

    struct w_cb_t {
        graphics_view_handler<platform::android>* self;
        void operator()(int v) const { self->apply_width(v); }
    };
    struct h_cb_t {
        graphics_view_handler<platform::android>* self;
        void operator()(int v) const { self->apply_height(v); }
    };

    jobject native_ = nullptr;

    w_cb_t                  w_cb_{this};
    h_cb_t                  h_cb_{this};
    signal_slot<const int&> w_slot_{};
    signal_slot<const int&> h_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_GRAPHICS_VIEW_HANDLER_HPP
