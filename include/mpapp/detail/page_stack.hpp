// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0014-page-navigation-stack.md
//
// `mpapp::detail::page_stack` — the host-agnostic stack engine that every
// page-level widget (navigation_page, tabbed_page-as-stack, flyout_page,
// shell) builds on top of. Owns:
//   - The page stack (std::vector<view*>, root-first, top last).
//   - Lifecycle signals fired around push/pop.
//   - An attached-property store ("child sets, host reads") keyed on view*.
//   - A hardware_back_requested signal that hosts wire to their native
//     equivalent (Android KEYCODE_BACK, Windows SystemNavigationManager,
//     manual GTK Back button).
//
// Sync mutators are the primitive; async variants live in the widget's
// public surface and wrap these. The mock handler tests this engine
// directly with no executor.

#ifndef MPAPP_DETAIL_PAGE_STACK_HPP
#define MPAPP_DETAIL_PAGE_STACK_HPP

#include <any>
#include <cstddef>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp::detail {

// Strongly-typed key for attached properties. Two different `attached_prop_key<T>`
// instances with the same `name` but different T are distinct — the type lives
// in the key, so the get/set path is type-safe at the call site even though
// the store is std::any underneath.
template <class T>
struct attached_prop_key {
    const char* name = "";
    // Identity for comparison.
    bool operator==(const attached_prop_key& other) const noexcept {
        return name == other.name || (name && other.name && std::string{name} == other.name);
    }
};

class page_stack {
public:
    page_stack() = default;
    ~page_stack() = default;

    page_stack(const page_stack&)            = delete;
    page_stack& operator=(const page_stack&) = delete;
    page_stack(page_stack&&)                 = delete;
    page_stack& operator=(page_stack&&)      = delete;

    // ----- Read-only stack state ----------------------------------------

    [[nodiscard]] std::size_t depth() const noexcept { return stack_.size(); }
    [[nodiscard]] bool        empty() const noexcept { return stack_.empty(); }

    [[nodiscard]] view* root() const noexcept {
        return stack_.empty() ? nullptr : stack_.front();
    }
    [[nodiscard]] view* top() const noexcept {
        return stack_.empty() ? nullptr : stack_.back();
    }
    [[nodiscard]] const std::vector<view*>& pages() const noexcept { return stack_; }

    // ----- Sync mutators (primitive per ADR-0014) -----------------------

    // Push `p` onto the top. Fires page_will_disappear(current_top),
    // page_will_appear(p), then mutates stack, then page_did_disappear
    // (previous top, or nullptr), page_did_appear(p).
    void push(view* p) {
        if (p == nullptr) return;
        view* prev = top();
        if (prev != nullptr) page_will_disappear.emit(prev);
        page_will_appear.emit(p);
        stack_.push_back(p);
        if (prev != nullptr) page_did_disappear.emit(prev);
        page_did_appear.emit(p);
    }

    // Pop the top. Returns the popped page (nullptr if stack was empty).
    // Fires the lifecycle signals for the page coming off and the page
    // newly exposed underneath. Does NOT clear that page's attached
    // properties (a popped page may be pushed again).
    view* pop() {
        if (stack_.empty()) return nullptr;
        view* leaving = stack_.back();
        view* incoming = stack_.size() >= 2 ? stack_[stack_.size() - 2] : nullptr;
        page_will_disappear.emit(leaving);
        if (incoming != nullptr) page_will_appear.emit(incoming);
        stack_.pop_back();
        page_did_disappear.emit(leaving);
        if (incoming != nullptr) page_did_appear.emit(incoming);
        return leaving;
    }

    // Pop everything except the root. Fires lifecycle for the disappearing
    // top and the newly-exposed root.
    void pop_to_root() {
        if (stack_.size() <= 1) return;
        view* leaving = stack_.back();
        view* incoming = stack_.front();
        page_will_disappear.emit(leaving);
        page_will_appear.emit(incoming);
        stack_.resize(1);
        page_did_disappear.emit(leaving);
        page_did_appear.emit(incoming);
    }

    // Insert `p` immediately before `before` in the stack. Does NOT fire
    // lifecycle signals (the visible top hasn't changed). If `before` is
    // not in the stack, this is a no-op.
    void insert_before(view* before, view* p) {
        if (p == nullptr || before == nullptr) return;
        for (auto it = stack_.begin(); it != stack_.end(); ++it) {
            if (*it == before) {
                stack_.insert(it, p);
                return;
            }
        }
    }

    // Remove `p` from the stack. If `p` is the current top, treat as pop().
    // If `p` is not in the stack, no-op.
    void remove(view* p) {
        if (p == nullptr || stack_.empty()) return;
        if (stack_.back() == p) { pop(); return; }
        for (auto it = stack_.begin(); it != stack_.end(); ++it) {
            if (*it == p) { stack_.erase(it); return; }
        }
    }

    // ----- Attached property store --------------------------------------
    // The "child sets, host reads" pattern: the child page sets a property
    // on itself (e.g. `set_has_back_button(child, true)`); the navigation
    // host reads that property when it goes to render the child's chrome.

    template <class T>
    void set_attached(view& v, const attached_prop_key<T>& key, T value) {
        attached_[&v][key.name] = std::any(std::move(value));
    }

    template <class T>
    [[nodiscard]] T get_attached(const view& v, const attached_prop_key<T>& key, T fallback = T{}) const {
        auto outer = attached_.find(const_cast<view*>(&v));
        if (outer == attached_.end()) return fallback;
        auto inner = outer->second.find(key.name);
        if (inner == outer->second.end()) return fallback;
        if (const T* p = std::any_cast<T>(&inner->second)) return *p;
        return fallback;
    }

    // Clear all attached props for a view (call when permanently removing
    // a view from the host).
    void clear_attached(view& v) { attached_.erase(&v); }

    // ----- Signals ------------------------------------------------------

    signal<view*> page_will_appear{};
    signal<view*> page_did_appear{};
    signal<view*> page_will_disappear{};
    signal<view*> page_did_disappear{};

    // Hardware back-button trigger. Each platform handler binds this to
    // its native equivalent and emits when the back gesture fires. The
    // engine itself does not handle this — page-level widgets subscribe
    // and call `pop()` when `depth() > 1`.
    signal<> hardware_back_requested{};

private:
    std::vector<view*>                                                 stack_{};
    std::unordered_map<view*, std::unordered_map<std::string, std::any>> attached_{};
};

} // namespace mpapp::detail

#endif // MPAPP_DETAIL_PAGE_STACK_HPP
