// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 window handler.
//
// `window_handler<platform::linux_>` wraps a `GtkApplicationWindow`. The
// underlying `GtkWidget*` is owned through a `void*` so this header
// stays GTK-free.

#ifndef MPAPP_HANDLERS_LINUX_WINDOW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_WINDOW_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../window.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <string>

namespace mpapp {

template <>
class window_handler<platform::linux_> {
public:
    window_handler();
    ~window_handler();

    window_handler(const window_handler&)            = delete;
    window_handler& operator=(const window_handler&) = delete;
    window_handler(window_handler&&)                 = delete;
    window_handler& operator=(window_handler&&)      = delete;

    void bind(window& w);

    // Native widget access — `void*` is a `GtkWidget*` to the underlying
    // GtkApplicationWindow. Type-erased so consumers don't need GTK.
    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

    // The window must be attached to a GtkApplication before it can be
    // shown. The application_handler calls this from inside the GTK
    // "activate" signal callback.
    void attach_to_application(void* gtk_application);

private:
    void apply_title(const std::string& v);
    void apply_content(view* v);
    void apply_width_or_height();
    void apply_is_visible(bool v);

    struct title_cb_t {
        window_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_title(v); }
    };
    struct content_cb_t {
        window_handler<platform::linux_>* self;
        void operator()(view* v) const { self->apply_content(v); }
    };
    struct width_cb_t {
        window_handler<platform::linux_>* self;
        void operator()(const int&) const { self->apply_width_or_height(); }
    };
    struct height_cb_t {
        window_handler<platform::linux_>* self;
        void operator()(const int&) const { self->apply_width_or_height(); }
    };
    struct visible_cb_t {
        window_handler<platform::linux_>* self;
        void operator()(bool v) const { self->apply_is_visible(v); }
    };

    void*   native_       = nullptr;  // GtkWidget*
    void*   gtk_app_      = nullptr;  // GtkApplication*
    window* bound_        = nullptr;

    title_cb_t                       title_cb_{this};
    content_cb_t                     content_cb_{this};
    width_cb_t                       width_cb_{this};
    height_cb_t                      height_cb_{this};
    visible_cb_t                     visible_cb_{this};
    signal_slot<const std::string&>  title_slot_{};
    signal_slot<view* const&>        content_slot_{};
    signal_slot<const int&>          width_slot_{};
    signal_slot<const int&>          height_slot_{};
    signal_slot<const bool&>         visible_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_WINDOW_HANDLER_HPP
