// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BindableLayout.md
//
// `mpapp::bindable_layout` — attached-property facility that turns any
// existing `layout` into a lightweight, non-virtualizing items host.
// Mirrors MAUI's `BindableLayout.GetItemsSource` / `SetItemsSource` and
// the surrounding `ItemTemplate` family.
//
// Mock surface (P2): the rich `items_source` / `data_template` types
// are stand-ins (`std::vector<std::string>` items, a single template
// callback). The real types land alongside `CollectionView` in M-03.
// The mapper-via-handler wiring is what mock tests exercise here — the
// shape of the API stays stable.

#ifndef MPAPP_BINDABLE_LAYOUT_HPP
#define MPAPP_BINDABLE_LAYOUT_HPP

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "layout.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

// Lightweight items source. Real implementations replace this with a
// type-erased range adapter that participates in INotifyCollectionChanged.
struct items_source {
    std::vector<std::string> items;

    bool operator==(const items_source& other) const = default;
};

// Lightweight data-template. The real `data_template` carries XAML
// template fragments and a clone() entry point; the mock just stores a
// labelled factory callback.
struct data_template {
    std::string name;
    std::function<std::shared_ptr<view>(const std::string&)> factory;

    bool empty() const noexcept { return !factory; }
};

template <class Platform>
class bindable_layout_handler;

// Attached-property carrier. Not instantiable as a control — all members
// are static, matching MAUI's BindableLayout static API.
class bindable_layout {
public:
    bindable_layout() = delete;

    // ----- Attached properties (XAML compiler emits calls to these) ---
    static void                  set_items_source(layout& host, items_source items);
    static const items_source&   get_items_source(const layout& host);

    static void                  set_item_template(layout& host, data_template tpl);
    static const data_template&  get_item_template(const layout& host);

    static void                  set_empty_view(layout& host, std::shared_ptr<view> empty);
    static std::shared_ptr<view> get_empty_view(const layout& host);

    // ----- Helper -----------------------------------------------------
    // Convenience for pure C++ wiring — sets items + template in one
    // call. Equivalent to setting both attached properties.
    static void enable(layout& host, items_source items, data_template item_template);

    // Releases attached state for `host`. Callers must invoke this from
    // the host's destructor in long-lived processes to avoid the static
    // side-table growing unboundedly. The mock layer does not auto-hook
    // `~layout()` because the real implementation (M-03) replaces this
    // pointer-keyed table with a per-host bindable-property store.
    static void detach(const layout& host);

private:
    struct attached_state {
        items_source          items{};
        data_template         item_template{};
        std::shared_ptr<view> empty_view{};
    };

    static std::unordered_map<const layout*, attached_state>& state();
    static attached_state&                                    state_for(const layout& host);
};

inline std::unordered_map<const layout*, bindable_layout::attached_state>&
bindable_layout::state() {
    // Static map keyed by host pointer. The handler-side mapper is the
    // primary observer of mutations; the map exists so attached-property
    // getters can return the same value setters wrote.
    static std::unordered_map<const layout*, attached_state> s;
    return s;
}

inline bindable_layout::attached_state&
bindable_layout::state_for(const layout& host) {
    return state()[&host];
}

inline void bindable_layout::set_items_source(layout& host, items_source items) {
    state_for(host).items = std::move(items);
}

inline const items_source&
bindable_layout::get_items_source(const layout& host) {
    return state_for(host).items;
}

inline void bindable_layout::set_item_template(layout& host, data_template tpl) {
    state_for(host).item_template = std::move(tpl);
}

inline const data_template&
bindable_layout::get_item_template(const layout& host) {
    return state_for(host).item_template;
}

inline void bindable_layout::set_empty_view(layout& host, std::shared_ptr<view> empty) {
    state_for(host).empty_view = std::move(empty);
}

inline std::shared_ptr<view>
bindable_layout::get_empty_view(const layout& host) {
    return state_for(host).empty_view;
}

inline void bindable_layout::enable(layout& host, items_source items, data_template item_template) {
    auto& s = state_for(host);
    s.items         = std::move(items);
    s.item_template = std::move(item_template);
}

inline void bindable_layout::detach(const layout& host) {
    state().erase(&host);
}

} // namespace mpapp

#endif // MPAPP_BINDABLE_LAYOUT_HPP
