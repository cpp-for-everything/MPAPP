// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_refresh_view handler.
//
// GTK4 has no native pull-to-refresh widget. The handler emulates one
// with a vertical `GtkBox` holding two children:
//   - a `GtkSpinner` above (hidden when not refreshing), and
//   - the wrapped scrollable content beneath.
//
// When `is_refreshing` flips true the spinner is shown + started; when
// false it is hidden + stopped. The native GtkWidget exposed to
// dispatch surfaces is the outer GtkBox.

#ifndef MPAPP_HANDLERS_LINUX_REFRESH_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_REFRESH_VIEW_HANDLER_HPP

#include <memory>
#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_refresh_view.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class refresh_view_handler<platform::linux_> {
public:
    refresh_view_handler();
    ~refresh_view_handler();

    refresh_view_handler(const refresh_view_handler&)            = delete;
    refresh_view_handler& operator=(const refresh_view_handler&) = delete;
    refresh_view_handler(refresh_view_handler&&)                 = delete;
    refresh_view_handler& operator=(refresh_view_handler&&)      = delete;

    void map_content(basic_refresh_view& r);
    void map_is_refreshing(basic_refresh_view& r);
    void map_refresh_color(basic_refresh_view& r);

    void bind_content(basic_refresh_view& r, view& child);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_is_refreshing(bool v);
    void apply_refresh_color(const brush_ref& b);

    struct content_cb_t       { refresh_view_handler<platform::linux_>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct is_refreshing_cb_t { refresh_view_handler<platform::linux_>* self; void operator()(bool v) const { self->apply_is_refreshing(v); } };
    struct refresh_color_cb_t { refresh_view_handler<platform::linux_>* self; void operator()(const brush_ref& b) const { self->apply_refresh_color(b); } };

    void* native_        = nullptr;  // GtkBox* (vertical)
    void* spinner_       = nullptr;  // GtkSpinner*
    void* current_child_ = nullptr;  // GtkWidget* — for removal

    void* provider_      = nullptr;  // GtkCssProvider* for spinner tint
    std::string class_name_{};

    content_cb_t                              content_cb_{this};
    is_refreshing_cb_t                        is_refreshing_cb_{this};
    refresh_color_cb_t                        refresh_color_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const bool&>                  is_refreshing_slot_{};
    signal_slot<const brush_ref&>             refresh_color_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_REFRESH_VIEW_HANDLER_HPP
