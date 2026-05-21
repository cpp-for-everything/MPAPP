// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 flyout_view handler.
//
// GTK4 has no native drawer widget. The handler builds a horizontal
// `GtkPaned` whose start child holds the flyout pane and end child
// holds the detail pane. `is_presented` toggles the visibility of
// the start child — collapsing the divider on close and revealing it
// on open.
//
// The native GtkWidget exposed to dispatch surfaces is the outer
// GtkPaned; the registrar at the bottom of the .cpp casts via
// `GTK_WIDGET()` for the registry callback.

#ifndef MPAPP_HANDLERS_LINUX_FLYOUT_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_FLYOUT_VIEW_HANDLER_HPP

#include <memory>

#include "../../flyout_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class flyout_view_handler<platform::linux_> {
public:
    flyout_view_handler();
    ~flyout_view_handler();

    flyout_view_handler(const flyout_view_handler&)            = delete;
    flyout_view_handler& operator=(const flyout_view_handler&) = delete;
    flyout_view_handler(flyout_view_handler&&)                 = delete;
    flyout_view_handler& operator=(flyout_view_handler&&)      = delete;

    void map_flyout(flyout_view& f);
    void map_detail(flyout_view& f);
    void map_is_presented(flyout_view& f);

    void bind_flyout(flyout_view& f, view& child);
    void bind_detail(flyout_view& f, view& child);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_flyout(const std::shared_ptr<view>& v);
    void apply_detail(const std::shared_ptr<view>& v);
    void apply_is_presented(bool v);

    struct flyout_cb_t       { flyout_view_handler<platform::linux_>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_flyout(v); } };
    struct detail_cb_t       { flyout_view_handler<platform::linux_>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_detail(v); } };
    struct is_presented_cb_t { flyout_view_handler<platform::linux_>* self; void operator()(bool v) const { self->apply_is_presented(v); } };

    void* native_         = nullptr;  // GtkPaned* (horizontal)
    void* current_flyout_ = nullptr;  // GtkWidget* — for removal
    void* current_detail_ = nullptr;  // GtkWidget* — for removal

    flyout_cb_t                               flyout_cb_{this};
    detail_cb_t                               detail_cb_{this};
    is_presented_cb_t                         is_presented_cb_{this};
    signal_slot<std::shared_ptr<view> const&> flyout_slot_{};
    signal_slot<std::shared_ptr<view> const&> detail_slot_{};
    signal_slot<const bool&>                  is_presented_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_FLYOUT_VIEW_HANDLER_HPP
