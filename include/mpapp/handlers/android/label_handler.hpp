// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android basic_label handler. Wraps
// `android.widget.TextView`.

#ifndef MPAPP_HANDLERS_ANDROID_LABEL_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_LABEL_HANDLER_HPP

#include "../../internal/basic_label.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

namespace mpapp::internal {

template <>
class label_handler<platform::android> {
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

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_label& /*x*/) noexcept {}


private:
    void apply_text(const std::string& text);
    void apply_font_size(double pt);
    void apply_typeface();   // derived from font_bold_ + font_family_

    struct text_callback {
        label_handler<platform::android>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct fsize_callback {
        label_handler<platform::android>* self = nullptr;
        void operator()(const double& v) const { self->apply_font_size(v); }
    };
    struct fbold_callback {
        label_handler<platform::android>* self = nullptr;
        void operator()(const bool& v) const { self->font_bold_ = v; self->apply_typeface(); }
    };
    struct ffamily_callback {
        label_handler<platform::android>* self = nullptr;
        void operator()(const std::string& v) const { self->font_family_ = v; self->apply_typeface(); }
    };

    jobject                          native_ = nullptr;  // global ref to TextView
    bool                             font_bold_ = false;
    std::string                      font_family_{};
    text_callback                    text_cb_{this};
    fsize_callback                   fsize_cb_{this};
    fbold_callback                   fbold_cb_{this};
    ffamily_callback                 ffamily_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
    signal_slot<const double&>       fsize_slot_{};
    signal_slot<const bool&>         fbold_slot_{};
    signal_slot<const std::string&>  ffamily_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_LABEL_HANDLER_HPP
