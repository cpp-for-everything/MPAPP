// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BindableLayout.md
//
// GTK4 `bindable_layout_handler<platform::linux_>` — keeps a vertical
// `GtkBox` in sync with the attached-property state recorded on the
// host `layout`. M-04b real surface: when `map_items_source(host)` is
// invoked the box is cleared and views resolved via the ADR-0013
// dispatch registry are re-appended. `item_template` is recorded but
// does not yet drive instantiation (deferred to the templating ADR).
//
// The handler is keyed on the host `layout&` (not on a `bindable_layout`
// instance — `bindable_layout` is a static facility with `= delete`
// constructor).

#ifndef MPAPP_HANDLERS_LINUX_BINDABLE_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_BINDABLE_LAYOUT_HANDLER_HPP

#include "../../bindable_layout.hpp"
#include "../../layout.hpp"
#include "../../platform.hpp"
#include "../../view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

extern "C" {
    typedef struct _GtkWidget GtkWidget;
}

namespace mpapp {

template <>
class bindable_layout_handler<platform::linux_> {
public:
    bindable_layout_handler();
    ~bindable_layout_handler();

    bindable_layout_handler(const bindable_layout_handler&)            = delete;
    bindable_layout_handler& operator=(const bindable_layout_handler&) = delete;
    bindable_layout_handler(bindable_layout_handler&&)                 = delete;
    bindable_layout_handler& operator=(bindable_layout_handler&&)      = delete;

    void map_items_source(layout& host);
    void map_item_template(layout& host);
    void map_empty_view(layout& host);

    // Vertical GtkBox owned by this handler — mirrors stack_layout's
    // child-append model.
    GtkWidget*       native() noexcept       { return native_; }
    const GtkWidget* native() const noexcept { return native_; }

private:
    void rebuild_children(layout& host);

    GtkWidget* native_ = nullptr;
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_BINDABLE_LAYOUT_HANDLER_HPP
