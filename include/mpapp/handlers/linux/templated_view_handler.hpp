// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 `basic_templated_view` handler — wraps `GtkBox` as a
// single-child container (no first-class `GtkBin` in GTK4). `template_id`
// is recorded into a member string; rendering still routes through
// `content` until the templating engine ADR lands.

#ifndef MPAPP_HANDLERS_LINUX_TEMPLATED_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_TEMPLATED_VIEW_HANDLER_HPP

#include <memory>
#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_templated_view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class templated_view_handler<platform::linux_> {
public:
    templated_view_handler();
    ~templated_view_handler();
    templated_view_handler(const templated_view_handler&)            = delete;
    templated_view_handler& operator=(const templated_view_handler&) = delete;
    templated_view_handler(templated_view_handler&&)                 = delete;
    templated_view_handler& operator=(templated_view_handler&&)      = delete;

    void map_content(basic_templated_view& t);
    void map_template_id(basic_templated_view& t);

    void bind_content(basic_templated_view& t, view& child);

    const std::string& template_id() const noexcept { return template_id_; }

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_templated_view& x);


private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_template_id(const std::string& v);

    struct content_cb_t     { templated_view_handler<platform::linux_>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct template_id_cb_t { templated_view_handler<platform::linux_>* self; void operator()(const std::string& v)            const { self->apply_template_id(v); } };

    void*       native_        = nullptr;  // GtkBox*
    void*       current_child_ = nullptr;  // GtkWidget* — for removal
    std::string template_id_{};

    content_cb_t                              content_cb_{this};
    template_id_cb_t                          template_id_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const std::string&>           template_id_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_TEMPLATED_VIEW_HANDLER_HPP
