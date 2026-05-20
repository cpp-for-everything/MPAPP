// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Toolbar.md
//
// `mpapp::toolbar` — page-scoped horizontal action bar with a title and a
// collection of `toolbar_item`s. Mirrors MAUI's `IToolbar` surface but
// stripped to the M-04b cross-platform subset (title + items collection).
// The richer MAUI surface (back-button affordance, drawer toggle,
// bar_background, bar_text_color, …) lands incrementally alongside the
// real handlers in M-05 / M-06.
//
// `items` is held as `Observable<std::vector<toolbar_item>>` rather than
// an observable collection because the M-04 binding-layer work hasn't
// landed the per-element child-template surface yet. Real handlers rebuild
// the native item list whenever the collection changes — see
// `vault/10_Architecture/Components/Picker.md` for the same pattern.

#ifndef MPAPP_TOOLBAR_HPP
#define MPAPP_TOOLBAR_HPP

#include <string>
#include <vector>

#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp {

// A single action in a toolbar. `icon` is a free-form string that real
// handlers interpret as a platform-specific resource (file path on
// Linux/Android, symbol name or path on Windows). An empty `icon`
// disables the icon slot. Equality is value-based — needed so Observable
// suppresses redundant change notifications when callers swap in an
// identical vector.
struct toolbar_item {
    std::string text;
    std::string icon;

    bool operator==(const toolbar_item&) const = default;
};

template <class Platform>
class toolbar_handler;

class toolbar : public view {
public:
    toolbar() = default;

    Observable<std::vector<toolbar_item>>  items{};
    Observable<std::string>                title{};

    toolbar_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const toolbar_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                      has_handler() const noexcept { return handler_ != nullptr; }
    void                                      set_handler(toolbar_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    toolbar_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_TOOLBAR_HPP
