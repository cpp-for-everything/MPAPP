// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_shape_view handler — wraps a GtkDrawingArea and renders into
// its cairo_t inside the `draw` callback. Hex color strings parse
// into RGBA values; the callback dispatches on `kind` and draws the
// appropriate cairo primitive. For v1, `polygon` and `path` render
// as the bounding rectangle pending the future graphics-backend ADR.

#ifndef MPAPP_HANDLERS_LINUX_SHAPE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_SHAPE_VIEW_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_shape_view.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class shape_view_handler<platform::linux_> {
public:
    shape_view_handler();
    ~shape_view_handler();

    shape_view_handler(const shape_view_handler&)            = delete;
    shape_view_handler& operator=(const shape_view_handler&) = delete;
    shape_view_handler(shape_view_handler&&)                 = delete;
    shape_view_handler& operator=(shape_view_handler&&)      = delete;

    void map_kind(basic_shape_view& s);
    void map_data(basic_shape_view& s);
    void map_fill(basic_shape_view& s);
    void map_stroke(basic_shape_view& s);
    void map_stroke_thickness(basic_shape_view& s);
    void map_opacity(basic_shape_view& s);

    void*       native() noexcept       { return native_; }   // GtkDrawingArea*
    const void* native() const noexcept { return native_; }

    // Used by the cairo draw callback to pull current state.
    basic_shape_view* bound() const noexcept { return bound_; }

private:
    void invalidate_();

    struct invalidate_cb_t {
        shape_view_handler<platform::linux_>* self;
        template <class T> void operator()(T const&) const { self->invalidate_(); }
    };

    void* native_ = nullptr;  // GtkDrawingArea*
    basic_shape_view* bound_ = nullptr;

    invalidate_cb_t                 kind_cb_{this};
    invalidate_cb_t                 data_cb_{this};
    invalidate_cb_t                 fill_cb_{this};
    invalidate_cb_t                 stroke_cb_{this};
    invalidate_cb_t                 stroke_thick_cb_{this};
    invalidate_cb_t                 opacity_cb_{this};
    signal_slot<const shape_kind&>  kind_slot_{};
    signal_slot<const std::string&> data_slot_{};
    signal_slot<const std::string&> fill_slot_{};
    signal_slot<const std::string&> stroke_slot_{};
    signal_slot<const double&>      stroke_thick_slot_{};
    signal_slot<const double&>      opacity_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_SHAPE_VIEW_HANDLER_HPP
