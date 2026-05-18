// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit label handler. Wraps UILabel.

#ifndef MPAPP_HANDLERS_IOS_LABEL_HANDLER_HPP
#define MPAPP_HANDLERS_IOS_LABEL_HANDLER_HPP

#include "../../label.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE

#include <string>

namespace mpapp {

template <>
class label_handler<platform::ios> {
public:
    label_handler();
    ~label_handler();

    label_handler(const label_handler&)            = delete;
    label_handler& operator=(const label_handler&) = delete;
    label_handler(label_handler&&)                 = delete;
    label_handler& operator=(label_handler&&)      = delete;

    void map_text(label& l);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_text(const std::string& text);

    struct text_callback {
        label_handler<platform::ios>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };

    void*                            native_ = nullptr;
    text_callback                    text_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
};

} // namespace mpapp

#  endif // TARGET_OS_IPHONE
#endif // __APPLE__
#endif // MPAPP_HANDLERS_IOS_LABEL_HANDLER_HPP
