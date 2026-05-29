// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TabbedPage.md
//                  vault/20_ADRs/ADR-0014-page-navigation-stack.md
//
// `mpapp::tabbed_page` — a Page that hosts multiple child Pages as tabs
// and shows one at a time. Distinct from `tabbed_view` (which lives
// inside a page); this IS a page and is intended to be the root of an
// app, possibly with each tab nesting a NavigationPage.
//
// Mock surface: children + selected_index + computed current_page +
// lifecycle on tab switch. Bar styling Observables exist as no-op
// holders so XAML compiles; real bar styling lands per-platform.

#ifndef MPAPP_INTERNAL_BASIC_TABBED_PAGE_HPP
#define MPAPP_INTERNAL_BASIC_TABBED_PAGE_HPP

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "../observable.hpp"
#include "basic_page.hpp"
#include "../platform.hpp"
#include "../signal.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class tabbed_page_handler;

class basic_tabbed_page : public internal::basic_page {
public:
    basic_tabbed_page() = default;
    ~basic_tabbed_page() override = default;

    basic_tabbed_page(const basic_tabbed_page&)            = delete;
    basic_tabbed_page& operator=(const basic_tabbed_page&) = delete;
    basic_tabbed_page(basic_tabbed_page&&)                 = delete;
    basic_tabbed_page& operator=(basic_tabbed_page&&)      = delete;

    // ----- Tab collection ----------------------------------------------

    Observable<std::vector<basic_page*>> children{};
    Observable<int>                selected_index{0};

    // Read-only: derived from children + selected_index.
    Observable<basic_page*>              current_page{nullptr};

    // ----- Mutators ----------------------------------------------------

    void add_tab(basic_page* p) {
        if (p == nullptr) return;
        auto v = children.get();
        v.push_back(p);
        children.set(std::move(v));
        if (selected_index.get() < 0 || selected_index.get() >= static_cast<int>(children.get().size())) {
            selected_index.set(0);
        }
        sync_current_page();
    }

    void remove_tab(basic_page* p) {
        auto v = children.get();
        auto it = std::find(v.begin(), v.end(), p);
        if (it == v.end()) return;
        int idx = static_cast<int>(it - v.begin());
        v.erase(it);
        children.set(std::move(v));
        // Clamp selection.
        int sel = selected_index.get();
        if (sel >= static_cast<int>(children.get().size())) {
            sel = static_cast<int>(children.get().size()) - 1;
        }
        if (sel < 0) sel = 0;
        selected_index.set(sel);
        sync_current_page();
        (void)idx;
    }

    void select(int idx) {
        if (idx < 0) idx = 0;
        if (idx >= static_cast<int>(children.get().size())) {
            idx = static_cast<int>(children.get().size()) - 1;
        }
        if (idx < 0) return;
        basic_page* old_current = current_page.get();
        basic_page* new_current = children.get()[static_cast<std::size_t>(idx)];
        if (old_current != new_current) {
            if (old_current != nullptr) tab_will_disappear.emit(old_current);
            tab_will_appear.emit(new_current);
        }
        selected_index.set(idx);
        sync_current_page();
        if (old_current != new_current) {
            if (old_current != nullptr) tab_did_disappear.emit(old_current);
            tab_did_appear.emit(new_current);
        }
    }

    // ----- Lifecycle signals -------------------------------------------

    signal<basic_page*> tab_will_appear{};
    signal<basic_page*> tab_did_appear{};
    signal<basic_page*> tab_will_disappear{};
    signal<basic_page*> tab_did_disappear{};

    // ----- Bar styling (mock-only no-op holders for XAML compile) ------

    Observable<std::string> bar_background_color{""};
    Observable<std::string> bar_text_color{""};
    Observable<std::string> selected_tab_color{""};
    Observable<std::string> unselected_tab_color{""};

    // ----- Handler -----------------------------------------------------

    tabbed_page_handler<platform::current>&       tp_handler() noexcept       { return *tp_handler_; }
    const tabbed_page_handler<platform::current>& tp_handler() const noexcept { return *tp_handler_; }
    bool                                          has_tp_handler() const noexcept { return tp_handler_ != nullptr; }
    void                                          set_tp_handler(tabbed_page_handler<platform::current>& h) noexcept { tp_handler_ = &h; }

private:
    void sync_current_page() {
        const auto& v = children.get();
        int idx = selected_index.get();
        basic_page* p = (idx >= 0 && idx < static_cast<int>(v.size())) ? v[static_cast<std::size_t>(idx)] : nullptr;
        if (current_page.get() != p) current_page.set(p);
    }

    tabbed_page_handler<platform::current>* tp_handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_TABBED_PAGE_HPP
