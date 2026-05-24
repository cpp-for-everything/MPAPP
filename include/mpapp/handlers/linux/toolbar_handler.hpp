// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_toolbar handler — wraps `GtkActionBar`.
//
// Each `toolbar_item` becomes a leading-side `GtkButton` packed via
// `gtk_action_bar_pack_start`; on rebuild, the existing children are
// removed first via `gtk_action_bar_remove`. `title` is rendered as a
// center-widget `GtkLabel`.

#ifndef MPAPP_HANDLERS_LINUX_TOOLBAR_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_TOOLBAR_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_toolbar.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class toolbar_handler<platform::linux_> {
public:
    toolbar_handler();
    ~toolbar_handler();
    toolbar_handler(const toolbar_handler&)            = delete;
    toolbar_handler& operator=(const toolbar_handler&) = delete;

    void map_items(basic_toolbar& t);
    void map_title(basic_toolbar& t);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_items(const std::vector<toolbar_item>& v);
    void apply_title(const std::string& v);

    struct items_cb_t { toolbar_handler<platform::linux_>* self; void operator()(const std::vector<toolbar_item>& v) const { self->apply_items(v); } };
    struct title_cb_t { toolbar_handler<platform::linux_>* self; void operator()(const std::string& v) const { self->apply_title(v); } };

    void* native_      = nullptr;   // GtkActionBar*
    void* title_label_ = nullptr;   // GtkLabel* (owned by the action bar's center slot)

    items_cb_t                                          items_cb_{this};
    title_cb_t                                          title_cb_{this};
    signal_slot<std::vector<toolbar_item> const&>       items_slot_{};
    signal_slot<const std::string&>                     title_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_TOOLBAR_HANDLER_HPP
