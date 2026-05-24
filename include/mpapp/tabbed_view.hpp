// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TabbedView.md
//
// `mpapp::tabbed_view` — embeddable tabs container with a horizontal
// tab bar and a single visible page region. This is the M-04b "real
// handlers on three platforms" landing of the widget; the mock surface
// is intentionally narrow — just `tab_titles` (the labels rendered on
// the tab strip) and `selected_index` (the active tab).
//
// The richer surface from the component doc (bar colors, child page
// templates, ItemsSource binding) lands in follow-ups alongside the
// `TabbedPage` page-level wiring and the templating engine ADR.

#ifndef MPAPP_TABBED_VIEW_HPP
#define MPAPP_TABBED_VIEW_HPP

#include <string>
#include <vector>

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform = platform::current>
class tabbed_view_handler;

class tabbed_view : public view {
public:
    tabbed_view() = default;

    // The tab labels rendered on the horizontal tab strip. On Windows
    // each label becomes a `mux::Controls::TabViewItem`; on Linux each
    // label is a `GtkLabel` attached to an empty page placeholder in a
    // `GtkNotebook`; on Android each label becomes a `TabHost` tab.
    // Adding/removing entries rebuilds the strip.
    Observable<std::vector<std::string>> tab_titles{};

    // The currently-selected tab index (0-based). -1 means "no
    // selection"; out-of-range values are clamped by the host control's
    // own semantics (most platforms silently ignore an invalid index).
    Observable<int>                      selected_index{-1};

    tabbed_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const tabbed_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                          has_handler() const noexcept { return handler_ != nullptr; }
    void                                          set_handler(tabbed_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    tabbed_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_TABBED_VIEW_HPP
