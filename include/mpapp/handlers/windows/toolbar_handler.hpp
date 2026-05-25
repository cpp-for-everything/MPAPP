// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_toolbar handler — wraps `mux::Controls::CommandBar`.
//
// Each `toolbar_item` becomes a `mux::Controls::AppBarButton` packed into
// the CommandBar's `PrimaryCommands` collection. The collection is rebuilt
// (cleared + repopulated) whenever the items Observable changes — the
// MAUI ToolbarExtensions pattern. `title` is surfaced via the CommandBar's
// `Content` slot (a leading TextBlock).

#ifndef MPAPP_HANDLERS_WINDOWS_TOOLBAR_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_TOOLBAR_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_toolbar.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class toolbar_handler<platform::windows> {
public:
    toolbar_handler();
    ~toolbar_handler();
    toolbar_handler(const toolbar_handler&)            = delete;
    toolbar_handler& operator=(const toolbar_handler&) = delete;

    void map_items(basic_toolbar& t);
    void map_title(basic_toolbar& t);

    winrt::Microsoft::UI::Xaml::Controls::CommandBar&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::CommandBar& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_toolbar& /*x*/) noexcept {}


private:
    void apply_items(const std::vector<toolbar_item>& v);
    void apply_title(const std::string& v);

    struct items_cb_t { toolbar_handler<platform::windows>* self; void operator()(const std::vector<toolbar_item>& v) const { self->apply_items(v); } };
    struct title_cb_t { toolbar_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_title(v); } };

    winrt::Microsoft::UI::Xaml::Controls::CommandBar native_{nullptr};

    items_cb_t                                          items_cb_{this};
    title_cb_t                                          title_cb_{this};
    signal_slot<std::vector<toolbar_item> const&>       items_slot_{};
    signal_slot<const std::string&>                     title_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_TOOLBAR_HANDLER_HPP
