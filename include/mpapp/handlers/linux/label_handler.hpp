// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 label handler.

#ifndef MPAPP_HANDLERS_LINUX_LABEL_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_LABEL_HANDLER_HPP

#include "../../label.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <string>

namespace mpapp {

template <>
class label_handler<platform::linux_> {
public:
    label_handler();
    ~label_handler();

    label_handler(const label_handler&)            = delete;
    label_handler& operator=(const label_handler&) = delete;
    label_handler(label_handler&&)                 = delete;
    label_handler& operator=(label_handler&&)      = delete;

    void map_text(label& l);

    // GtkWidget* (GtkLabel), type-erased.
    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_text(const std::string& text);

    struct text_callback {
        label_handler<platform::linux_>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };

    void*                            native_ = nullptr; // GtkWidget*
    text_callback                    text_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_LABEL_HANDLER_HPP
