// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 basic_label handler.

#ifndef MPAPP_HANDLERS_LINUX_LABEL_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_LABEL_HANDLER_HPP

#include "../../internal/basic_label.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <string>

namespace mpapp::internal {

template <>
class label_handler<platform::linux_> {
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

    // GtkWidget* (GtkLabel), type-erased.
    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_label& x);


private:
    void apply_text(const std::string& text);
    void apply_font();   // rebuilds the PangoAttrList from stored font state

    struct text_callback {
        label_handler<platform::linux_>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct fsize_callback {
        label_handler<platform::linux_>* self = nullptr;
        void operator()(const double& v) const { self->font_size_ = v; self->apply_font(); }
    };
    struct fbold_callback {
        label_handler<platform::linux_>* self = nullptr;
        void operator()(const bool& v) const { self->font_bold_ = v; self->apply_font(); }
    };
    struct ffamily_callback {
        label_handler<platform::linux_>* self = nullptr;
        void operator()(const std::string& v) const { self->font_family_ = v; self->apply_font(); }
    };

    void*                            native_ = nullptr; // GtkWidget*
    double                           font_size_   = 0.0;
    bool                             font_bold_   = false;
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
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_LABEL_HANDLER_HPP
