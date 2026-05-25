// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_content_page handler — wraps a vertical `GtkBox`:
//   - top child: a `GtkLabel` carrying `title`
//   - bottom child: a single-child host for `content`
// Padding is applied via `gtk_widget_set_margin_*` on the outer box.

#ifndef MPAPP_HANDLERS_LINUX_CONTENT_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_CONTENT_PAGE_HANDLER_HPP

#include <memory>
#include <string>

#include "../../internal/basic_content_page.hpp"
#include "../../layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class content_page_handler<platform::linux_> {
public:
    content_page_handler();
    ~content_page_handler();

    content_page_handler(const content_page_handler&)            = delete;
    content_page_handler& operator=(const content_page_handler&) = delete;
    content_page_handler(content_page_handler&&)                 = delete;
    content_page_handler& operator=(content_page_handler&&)      = delete;

    void map_title(basic_content_page& p);
    void map_content(basic_content_page& p);
    void map_padding(basic_content_page& p);

    void bind_content(basic_content_page& p, view& child);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_content_page& x);


private:
    void apply_title(const std::string& v);
    void apply_content(const std::shared_ptr<view>& v);
    void apply_padding(const thickness& t);

    struct title_cb_t   { content_page_handler<platform::linux_>* self; void operator()(const std::string& v) const { self->apply_title(v); } };
    struct content_cb_t { content_page_handler<platform::linux_>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct padding_cb_t { content_page_handler<platform::linux_>* self; void operator()(const thickness& t) const { self->apply_padding(t); } };

    void* native_        = nullptr;  // GtkBox* (vertical)
    void* title_label_   = nullptr;  // GtkLabel*
    void* current_child_ = nullptr;  // GtkWidget* — for removal

    title_cb_t                                title_cb_{this};
    content_cb_t                              content_cb_{this};
    padding_cb_t                              padding_cb_{this};
    signal_slot<const std::string&>           title_slot_{};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const thickness&>             padding_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_CONTENT_PAGE_HANDLER_HPP
