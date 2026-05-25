// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_view_cell handler — vertical GtkBox single-child host with row
// padding margins; child resolved via ADR-0013 dispatch.

#ifndef MPAPP_HANDLERS_LINUX_VIEW_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_VIEW_CELL_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_view_cell.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class view_cell_handler<platform::linux_> {
public:
    view_cell_handler();
    ~view_cell_handler();

    view_cell_handler(const view_cell_handler&)            = delete;
    view_cell_handler& operator=(const view_cell_handler&) = delete;
    view_cell_handler(view_cell_handler&&)                 = delete;
    view_cell_handler& operator=(view_cell_handler&&)      = delete;

    void map_content(basic_view_cell& c);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_view_cell& x);


private:
    void apply_content(view* v);

    struct content_cb_t {
        view_cell_handler<platform::linux_>* self;
        void operator()(view* v) const { self->apply_content(v); }
    };

    void* native_        = nullptr;  // GtkBox (vertical, single slot)
    void* current_child_ = nullptr;  // currently-attached GtkWidget*

    content_cb_t content_cb_{this};
    signal_slot<view* const&> content_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_VIEW_CELL_HANDLER_HPP
