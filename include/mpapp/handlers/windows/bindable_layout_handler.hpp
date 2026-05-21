// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BindableLayout.md
//
// WinUI 3 `bindable_layout_handler<platform::windows>` — keeps a
// `mux::Controls::StackPanel` in sync with the attached-property state
// the framework records on the host `layout`. M-04b real surface is
// simpler than MAUI's: when `map_items_source(host)` is invoked the
// handler clears the panel's children and re-appends views resolved via
// the ADR-0013 dispatch registry. `item_template` is recorded but does
// not yet drive instantiation (deferred to the templating ADR — same
// shape as templated_view).
//
// The handler is keyed on the host `layout&` (not on a `bindable_layout`
// instance, which is a static facility with `= delete` constructor).

#ifndef MPAPP_HANDLERS_WINDOWS_BINDABLE_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_BINDABLE_LAYOUT_HANDLER_HPP

#include <memory>

#include "../../bindable_layout.hpp"
#include "../../layout.hpp"
#include "../../platform.hpp"
#include "../../view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class bindable_layout_handler<platform::windows> {
public:
    bindable_layout_handler();
    ~bindable_layout_handler();

    bindable_layout_handler(const bindable_layout_handler&)            = delete;
    bindable_layout_handler& operator=(const bindable_layout_handler&) = delete;
    bindable_layout_handler(bindable_layout_handler&&)                 = delete;
    bindable_layout_handler& operator=(bindable_layout_handler&&)      = delete;

    // Mappers — invoked by the framework when the corresponding attached
    // property changes on `host`. The mock surface fires these as a
    // one-shot snapshot; the real surface does the same, plus rebuilds
    // the panel's Children on `map_items_source`.
    void map_items_source(layout& host);
    void map_item_template(layout& host);
    void map_empty_view(layout& host);

    // The handler's own native container — a vertical StackPanel that
    // mirrors stack_layout's add/remove model. Exposed to dispatch
    // surfaces (and to spikes that want to host the panel directly).
    winrt::Microsoft::UI::Xaml::Controls::StackPanel&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::StackPanel& native() const noexcept { return native_; }

private:
    void rebuild_children(layout& host);

    winrt::Microsoft::UI::Xaml::Controls::StackPanel native_{nullptr};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_BINDABLE_LAYOUT_HANDLER_HPP
