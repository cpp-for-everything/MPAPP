// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_page handler — wraps a vertical `GtkBox`:
//   - top child: a `GtkLabel` carrying `title`
//   - middle child: a single-child host for `content`
//   - overlay: a `GtkSpinner` toggled by `is_busy`
// The underlying `GtkWidget*` is held as `void*` so this header stays
// GTK-free (mirrors `content_page_handler<platform::linux_>`).

#ifndef MPAPP_HANDLERS_LINUX_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_PAGE_HANDLER_HPP

#include <string>

#include "../../internal/basic_page.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class page_handler<platform::linux_> {
public:
    page_handler();
    ~page_handler();

    page_handler(const page_handler&)            = delete;
    page_handler& operator=(const page_handler&) = delete;
    page_handler(page_handler&&)                 = delete;
    page_handler& operator=(page_handler&&)      = delete;

    void map_title(basic_page& p);
    void map_content(basic_page& p);
    void map_is_busy(basic_page& p);

    void bind_content(basic_page& p, view& child);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_title(const std::string& v);
    void apply_content(view* v);
    void apply_is_busy(bool v);

    struct title_cb_t   { page_handler<platform::linux_>* self; void operator()(const std::string& v) const { self->apply_title(v); } };
    struct content_cb_t { page_handler<platform::linux_>* self; void operator()(view* v) const { self->apply_content(v); } };
    struct busy_cb_t    { page_handler<platform::linux_>* self; void operator()(bool v) const { self->apply_is_busy(v); } };

    void* native_        = nullptr;  // GtkBox* (vertical)
    void* title_label_   = nullptr;  // GtkLabel*
    void* busy_spinner_  = nullptr;  // GtkSpinner*
    void* current_child_ = nullptr;  // GtkWidget* — current content (for removal)

    title_cb_t                       title_cb_{this};
    content_cb_t                     content_cb_{this};
    busy_cb_t                        busy_cb_{this};
    signal_slot<const std::string&>  title_slot_{};
    signal_slot<view* const&>        content_slot_{};
    signal_slot<const bool&>         busy_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_PAGE_HANDLER_HPP
