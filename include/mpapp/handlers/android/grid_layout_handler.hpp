// SPDX-License-Identifier: Apache-2.0
// Android grid_layout handler. Wraps android.widget.GridLayout. Per-child
// placement uses GridLayout.LayoutParams (row, column, rowSpan, columnSpan).

#ifndef MPAPP_HANDLERS_ANDROID_GRID_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_GRID_LAYOUT_HANDLER_HPP

#include <vector>

#include "../../grid_layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class grid_layout_handler<platform::android> {
public:
    grid_layout_handler();
    ~grid_layout_handler();

    grid_layout_handler(const grid_layout_handler&)            = delete;
    grid_layout_handler& operator=(const grid_layout_handler&) = delete;
    grid_layout_handler(grid_layout_handler&&)                 = delete;
    grid_layout_handler& operator=(grid_layout_handler&&)      = delete;

    void map_row_definitions(grid_layout& g);
    void map_column_definitions(grid_layout& g);
    void map_row_spacing(grid_layout& g);
    void map_column_spacing(grid_layout& g);

    void add_child(grid_layout& g, view& child);

    jobject native() const noexcept { return native_; }

private:
    void apply_row_count(int n);
    void apply_column_count(int n);
    void apply_row_spacing(double s);
    void apply_column_spacing(double s);

    struct rows_cb_t {
        grid_layout_handler<platform::android>* self;
        void operator()(const std::vector<track_def>& v) const {
            self->apply_row_count(static_cast<int>(v.size()));
        }
    };
    struct cols_cb_t {
        grid_layout_handler<platform::android>* self;
        void operator()(const std::vector<track_def>& v) const {
            self->apply_column_count(static_cast<int>(v.size()));
        }
    };
    struct rsp_cb_t {
        grid_layout_handler<platform::android>* self;
        void operator()(double s) const { self->apply_row_spacing(s); }
    };
    struct csp_cb_t {
        grid_layout_handler<platform::android>* self;
        void operator()(double s) const { self->apply_column_spacing(s); }
    };

    jobject native_ = nullptr;  // android.widget.GridLayout

    rows_cb_t rows_cb_{this};
    cols_cb_t cols_cb_{this};
    rsp_cb_t  rsp_cb_{this};
    csp_cb_t  csp_cb_{this};
    signal_slot<const std::vector<track_def>&> rows_slot_{};
    signal_slot<const std::vector<track_def>&> cols_slot_{};
    signal_slot<const double&>                  rsp_slot_{};
    signal_slot<const double&>                  csp_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_GRID_LAYOUT_HANDLER_HPP
