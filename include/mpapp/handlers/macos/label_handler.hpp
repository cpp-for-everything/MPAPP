// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — AppKit basic_label handler. Wraps NSTextField in
// basic_label mode (`bezeled=NO`, `editable=NO`, `selectable=NO`,
// `drawsBackground=NO`).

#ifndef MPAPP_HANDLERS_MACOS_LABEL_HANDLER_HPP
#define MPAPP_HANDLERS_MACOS_LABEL_HANDLER_HPP

#include "../../internal/basic_label.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  if !TARGET_OS_IPHONE

#include <string>

namespace mpapp::internal {

template <>
class label_handler<platform::macos> {
public:
    label_handler();
    ~label_handler();

    label_handler(const label_handler&)            = delete;
    label_handler& operator=(const label_handler&) = delete;
    label_handler(label_handler&&)                 = delete;
    label_handler& operator=(label_handler&&)      = delete;

    void map_text(basic_label& l);

    // NSTextField*, retained, type-erased.
    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_label& /*x*/) noexcept {}


private:
    void apply_text(const std::string& text);

    struct text_callback {
        label_handler<platform::macos>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };

    void*                            native_ = nullptr;
    text_callback                    text_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
};

} // namespace mpapp::internal
#  endif // !TARGET_OS_IPHONE
#endif // __APPLE__
#endif // MPAPP_HANDLERS_MACOS_LABEL_HANDLER_HPP
