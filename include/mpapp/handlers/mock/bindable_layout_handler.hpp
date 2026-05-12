// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BindableLayout.md
//
// `bindable_layout_handler<platform::mock>` — records attached-property
// mapper calls. BindableLayout has no platform widget of its own; the
// mock handler captures the call sequence the framework would forward
// to the host layout's children.

#ifndef MPAPP_HANDLERS_MOCK_BINDABLE_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_BINDABLE_LAYOUT_HANDLER_HPP

#include <cstddef>

#include "../../bindable_layout.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class bindable_layout_handler<platform::mock>
    : public mock_handler_base<bindable_layout_handler<platform::mock>, layout> {
public:
    bindable_layout_handler() = default;

    // Record the current state of the attached properties for `host`.
    // Unlike the per-`view` Observable bindings used by other mock
    // handlers, BindableLayout state lives in a static side-table —
    // there's no `changed` signal to subscribe to, so the mapper just
    // takes a one-shot snapshot. Tests call the same mapper repeatedly
    // to observe propagated mutations.

    void map_items_source(layout& host) {
        const auto& src = bindable_layout::get_items_source(host);
        record("items_source.count", src.items.size());
    }

    void map_item_template(layout& host) {
        const auto& tpl = bindable_layout::get_item_template(host);
        record("item_template", tpl.name);
    }

    void map_empty_view(layout& host) {
        auto ev = bindable_layout::get_empty_view(host);
        record("empty_view.present", ev != nullptr);
    }
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_BINDABLE_LAYOUT_HANDLER_HPP
