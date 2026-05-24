// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_content_view handler — wraps `GtkBox` as a single-child container.

#ifndef MPAPP_HANDLERS_LINUX_CONTENT_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_CONTENT_VIEW_HANDLER_HPP

#include <memory>

#include "../../internal/basic_content_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class content_view_handler<platform::linux_> {
public:
    content_view_handler();
    ~content_view_handler();
    content_view_handler(const content_view_handler&)            = delete;
    content_view_handler& operator=(const content_view_handler&) = delete;

    void map_content(basic_content_view& c);
    void bind_content(basic_content_view& c, view& child);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_content(const std::shared_ptr<view>& v);

    struct content_cb_t { content_view_handler<platform::linux_>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };

    void* native_        = nullptr;
    void* current_child_ = nullptr;

    content_cb_t                              content_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_CONTENT_VIEW_HANDLER_HPP
