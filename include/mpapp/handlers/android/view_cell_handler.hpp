// SPDX-License-Identifier: Apache-2.0
// Android view_cell handler — FrameLayout host with row padding; child
// resolved via ADR-0013 dispatch.

#ifndef MPAPP_HANDLERS_ANDROID_VIEW_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_VIEW_CELL_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view_cell.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class view_cell_handler<platform::android> {
public:
    view_cell_handler();
    ~view_cell_handler();

    view_cell_handler(const view_cell_handler&)            = delete;
    view_cell_handler& operator=(const view_cell_handler&) = delete;
    view_cell_handler(view_cell_handler&&)                 = delete;
    view_cell_handler& operator=(view_cell_handler&&)      = delete;

    void map_content(view_cell& c);

    jobject native() const noexcept { return native_; }

private:
    void apply_content(view* v);

    struct content_cb_t {
        view_cell_handler<platform::android>* self;
        void operator()(view* v) const { self->apply_content(v); }
    };

    jobject native_ = nullptr;   // FrameLayout

    content_cb_t content_cb_{this};
    signal_slot<view* const&> content_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_VIEW_CELL_HANDLER_HPP
