// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_title_bar handler — wraps `GtkHeaderBar`. The
// header bar's title-widget slot hosts a `GtkLabel` for the title and a
// second `GtkLabel` packed underneath in a vertical box for the subtitle
// (GTK4 dropped the built-in subtitle slot that GtkHeaderBar had on
// GTK3, so we recreate the layout manually).

#ifndef MPAPP_HANDLERS_LINUX_TITLE_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_TITLE_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_title_bar.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <string>

namespace mpapp::internal {

template <>
class title_bar_handler<platform::linux_> {
public:
    title_bar_handler();
    ~title_bar_handler();
    title_bar_handler(const title_bar_handler&)            = delete;
    title_bar_handler& operator=(const title_bar_handler&) = delete;
    title_bar_handler(title_bar_handler&&)                 = delete;
    title_bar_handler& operator=(title_bar_handler&&)      = delete;

    void map_title(basic_title_bar& t);
    void map_subtitle(basic_title_bar& t);

    // GtkWidget* (the GtkHeaderBar), type-erased.
    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_title(const std::string& v);
    void apply_subtitle(const std::string& v);

    struct title_cb_t    { title_bar_handler<platform::linux_>* self; void operator()(const std::string& v) const { self->apply_title(v); } };
    struct subtitle_cb_t { title_bar_handler<platform::linux_>* self; void operator()(const std::string& v) const { self->apply_subtitle(v); } };

    void* native_         = nullptr; // GtkHeaderBar (GtkWidget*)
    void* title_label_    = nullptr; // GtkLabel inside the title-widget box
    void* subtitle_label_ = nullptr; // GtkLabel beneath the title

    title_cb_t                       title_cb_{this};
    subtitle_cb_t                    subtitle_cb_{this};
    signal_slot<const std::string&>  title_slot_{};
    signal_slot<const std::string&>  subtitle_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_TITLE_BAR_HANDLER_HPP
