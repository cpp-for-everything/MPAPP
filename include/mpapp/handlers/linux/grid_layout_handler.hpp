// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_grid_layout handler. GtkGrid handles layout natively; mpapp
// configures spacing + per-child attach via gtk_grid_attach.
//
// Note: GtkGrid track sizing is implicit (no explicit row/column
// definitions). track_def vectors influence track count only; Auto /
// Star / Fixed shapes degrade to "natural sizing" — GtkGrid honors
// hexpand/vexpand on children to approximate Star behavior. A future
// follow-up could wire GtkBoxLayout for explicit star sizing.

#ifndef MPAPP_HANDLERS_LINUX_GRID_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_GRID_LAYOUT_HANDLER_HPP

#include <vector>

#include "../../internal/basic_grid_layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class grid_layout_handler<platform::linux_> {
public:
    grid_layout_handler();
    ~grid_layout_handler();

    grid_layout_handler(const grid_layout_handler&)            = delete;
    grid_layout_handler& operator=(const grid_layout_handler&) = delete;
    grid_layout_handler(grid_layout_handler&&)                 = delete;
    grid_layout_handler& operator=(grid_layout_handler&&)      = delete;

    void map_row_definitions(basic_grid_layout& g);
    void map_column_definitions(basic_grid_layout& g);
    void map_row_spacing(basic_grid_layout& g);
    void map_column_spacing(basic_grid_layout& g);

    void add_child(basic_grid_layout& g, view& child);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_grid_layout& x);


private:
    void apply_row_spacing(double s);
    void apply_column_spacing(double s);
    void rebuild_dummy_rows(const std::vector<track_def>& v);
    void rebuild_dummy_columns(const std::vector<track_def>& v);

    struct rows_cb_t {
        grid_layout_handler<platform::linux_>* self;
        void operator()(const std::vector<track_def>& v) const { self->rebuild_dummy_rows(v); }
    };
    struct cols_cb_t {
        grid_layout_handler<platform::linux_>* self;
        void operator()(const std::vector<track_def>& v) const { self->rebuild_dummy_columns(v); }
    };
    struct rsp_cb_t {
        grid_layout_handler<platform::linux_>* self;
        void operator()(double s) const { self->apply_row_spacing(s); }
    };
    struct csp_cb_t {
        grid_layout_handler<platform::linux_>* self;
        void operator()(double s) const { self->apply_column_spacing(s); }
    };

    void* native_ = nullptr;  // GtkGrid*

    rows_cb_t rows_cb_{this};
    cols_cb_t cols_cb_{this};
    rsp_cb_t  rsp_cb_{this};
    csp_cb_t  csp_cb_{this};
    signal_slot<const std::vector<track_def>&> rows_slot_{};
    signal_slot<const std::vector<track_def>&> cols_slot_{};
    signal_slot<const double&>                  rsp_slot_{};
    signal_slot<const double&>                  csp_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_GRID_LAYOUT_HANDLER_HPP
