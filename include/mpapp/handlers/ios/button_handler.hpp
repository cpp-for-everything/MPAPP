// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit button handler. Wraps UIButton.

#ifndef MPAPP_HANDLERS_IOS_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_IOS_BUTTON_HANDLER_HPP

#include "../../button.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE

#include <string>

namespace mpapp {

template <>
class button_handler<platform::ios> {
public:
    button_handler();
    ~button_handler();

    button_handler(const button_handler&)            = delete;
    button_handler& operator=(const button_handler&) = delete;
    button_handler(button_handler&&)                 = delete;
    button_handler& operator=(button_handler&&)      = delete;

    void map_text(button& b);
    void map_clicked(button& b);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_text(const std::string& text);

    struct text_callback {
        button_handler<platform::ios>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };

    void*                            native_ = nullptr;
    void*                            target_ = nullptr;
    text_callback                    text_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
};

} // namespace mpapp

#  endif // TARGET_OS_IPHONE
#endif // __APPLE__
#endif // MPAPP_HANDLERS_IOS_BUTTON_HANDLER_HPP
