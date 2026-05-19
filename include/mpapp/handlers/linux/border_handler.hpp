// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 border handler — wraps `GtkBox` as a single-child
// container, applies stroke / corners / padding through a per-handler
// CSS provider scoped via a unique class name.

#ifndef MPAPP_HANDLERS_LINUX_BORDER_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_BORDER_HANDLER_HPP

#include <memory>
#include <string>

#include "../../border.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class border_handler<platform::linux_> {
public:
    border_handler();
    ~border_handler();

    border_handler(const border_handler&)            = delete;
    border_handler& operator=(const border_handler&) = delete;
    border_handler(border_handler&&)                 = delete;
    border_handler& operator=(border_handler&&)      = delete;

    void map_content(border& b);
    void map_padding(border& b);
    void map_stroke(border& b);
    void map_stroke_thickness(border& b);
    void map_stroke_shape(border& b);

    void bind_content(border& b, view& child);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_padding(const thickness& t);
    void apply_stroke(const brush_ref& b);
    void apply_stroke_thickness(double t);
    void apply_stroke_shape(const stroke_shape_desc& s);
    void reload_css();

    struct content_cb_t       { border_handler<platform::linux_>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct padding_cb_t       { border_handler<platform::linux_>* self; void operator()(const thickness& t) const { self->apply_padding(t); } };
    struct stroke_cb_t        { border_handler<platform::linux_>* self; void operator()(const brush_ref& b) const { self->apply_stroke(b); } };
    struct stroke_thick_cb_t  { border_handler<platform::linux_>* self; void operator()(double t) const { self->apply_stroke_thickness(t); } };
    struct stroke_shape_cb_t  { border_handler<platform::linux_>* self; void operator()(const stroke_shape_desc& s) const { self->apply_stroke_shape(s); } };

    void* native_   = nullptr;  // GtkBox*
    void* provider_ = nullptr;  // GtkCssProvider*
    std::string class_name_{};
    void* current_child_ = nullptr;  // GtkWidget* — for removal

    brush_ref            cached_stroke_{};
    double               cached_stroke_thickness_ = 1.0;
    stroke_shape_desc    cached_stroke_shape_{};
    thickness            cached_padding_{};

    content_cb_t                              content_cb_{this};
    padding_cb_t                              padding_cb_{this};
    stroke_cb_t                               stroke_cb_{this};
    stroke_thick_cb_t                         stroke_thick_cb_{this};
    stroke_shape_cb_t                         stroke_shape_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const thickness&>             padding_slot_{};
    signal_slot<const brush_ref&>             stroke_slot_{};
    signal_slot<const double&>                stroke_thick_slot_{};
    signal_slot<const stroke_shape_desc&>     stroke_shape_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_BORDER_HANDLER_HPP
