// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 button handler.

#ifndef MPAPP_HANDLERS_LINUX_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_BUTTON_HANDLER_HPP

#include "../../button.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <string>

namespace mpapp {

template <>
class button_handler<platform::linux_> {
public:
    button_handler();
    ~button_handler();

    button_handler(const button_handler&)            = delete;
    button_handler& operator=(const button_handler&) = delete;
    button_handler(button_handler&&)                 = delete;
    button_handler& operator=(button_handler&&)      = delete;

    void map_text(button& b);
    void map_clicked(button& b);

    // GtkWidget* (GtkButton), type-erased.
    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_text(const std::string& text);

    struct text_callback {
        button_handler<platform::linux_>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };

    void*                            native_      = nullptr; // GtkWidget*
    button*                          bound_       = nullptr;
    unsigned long                    click_handler_id_ = 0;  // gulong from g_signal_connect
    text_callback                    text_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_BUTTON_HANDLER_HPP
