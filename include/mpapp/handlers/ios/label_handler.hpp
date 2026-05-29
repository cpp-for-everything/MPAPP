// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit basic_label handler. Wraps UILabel.

#ifndef MPAPP_HANDLERS_IOS_LABEL_HANDLER_HPP
#define MPAPP_HANDLERS_IOS_LABEL_HANDLER_HPP

#include "../../internal/basic_label.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE

#include <string>

namespace mpapp::internal {

template <>
class label_handler<platform::ios> {
public:
    label_handler();
    ~label_handler();

    label_handler(const label_handler&)            = delete;
    label_handler& operator=(const label_handler&) = delete;
    label_handler(label_handler&&)                 = delete;
    label_handler& operator=(label_handler&&)      = delete;

    void map_text(basic_label& l);
    void map_font_size(basic_label& l);
    void map_font_bold(basic_label& l);
    void map_font_family(basic_label& l);
    void map_text_color(basic_label& l);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_label& /*x*/) noexcept {}


private:
    void apply_text(const std::string& text);
    void apply_font();   // rebuilds UIFont from stored font state

    struct text_callback {
        label_handler<platform::ios>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct fsize_callback {
        label_handler<platform::ios>* self = nullptr;
        void operator()(const double& v) const { self->font_size_ = v; self->apply_font(); }
    };
    struct fbold_callback {
        label_handler<platform::ios>* self = nullptr;
        void operator()(const bool& v) const { self->font_bold_ = v; self->apply_font(); }
    };
    struct ffamily_callback {
        label_handler<platform::ios>* self = nullptr;
        void operator()(const std::string& v) const { self->font_family_ = v; self->apply_font(); }
    };
    struct tcolor_callback {
        label_handler<platform::ios>* self = nullptr;
        void operator()(const color& v) const { self->text_color_ = v; self->apply_font(); }
    };

    void*                            native_ = nullptr;
    double                           font_size_   = 0.0;
    bool                             font_bold_   = false;
    std::string                      font_family_{};
    color                            text_color_{0.0, 0.0, 0.0, 0.0};
    text_callback                    text_cb_{this};
    fsize_callback                   fsize_cb_{this};
    fbold_callback                   fbold_cb_{this};
    ffamily_callback                 ffamily_cb_{this};
    tcolor_callback                  tcolor_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
    signal_slot<const double&>       fsize_slot_{};
    signal_slot<const bool&>         fbold_slot_{};
    signal_slot<const std::string&>  ffamily_slot_{};
    signal_slot<const color&>        tcolor_slot_{};
};

} // namespace mpapp::internal
#  endif // TARGET_OS_IPHONE
#endif // __APPLE__
#endif // MPAPP_HANDLERS_IOS_LABEL_HANDLER_HPP
