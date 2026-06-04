// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0014-page-navigation-stack.md
//
// `mpapp::detail::modal_stack` — the host-agnostic modal navigation engine.
// Mirrors the structure of `detail::page_stack` but for MAUI's modal stack
// (`Navigation.PushModalAsync` / `PopModalAsync`). A modal sits *above* the
// regular navigation stack and is dismissed in LIFO order.
//
// Owns:
//   - The modal page stack (std::vector<view*>, bottom-first, top last).
//   - `modal_pushed` / `modal_popped` signals fired around push/pop.
//
// Sync mutators are the primitive; async variants live in the public modal
// surface. The mock handler tests this engine directly with no executor.

#ifndef MPAPP_DETAIL_MODAL_STACK_HPP
#define MPAPP_DETAIL_MODAL_STACK_HPP

#include <cstddef>
#include <vector>

#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp::detail {

class modal_stack {
public:
    modal_stack() = default;
    ~modal_stack() = default;

    modal_stack(const modal_stack&)            = delete;
    modal_stack& operator=(const modal_stack&) = delete;
    modal_stack(modal_stack&&)                 = delete;
    modal_stack& operator=(modal_stack&&)      = delete;

    // ----- Read-only stack state ----------------------------------------

    [[nodiscard]] std::size_t depth() const noexcept { return stack_.size(); }
    [[nodiscard]] bool        empty() const noexcept { return stack_.empty(); }

    [[nodiscard]] view* current() const noexcept {
        return stack_.empty() ? nullptr : stack_.back();
    }
    [[nodiscard]] const std::vector<view*>& pages() const noexcept { return stack_; }

    // ----- Sync mutators (primitive) ------------------------------------

    // Push `p` as the new top modal. Fires `modal_pushed(p)` after the
    // mutation. A null page is ignored.
    void push(view* p) {
        if (p == nullptr) return;
        stack_.push_back(p);
        modal_pushed.emit(p);
    }

    // Pop the top modal. Returns the popped page (nullptr if the stack was
    // empty). Fires `modal_popped(leaving)` after the mutation.
    view* pop() {
        if (stack_.empty()) return nullptr;
        view* leaving = stack_.back();
        stack_.pop_back();
        modal_popped.emit(leaving);
        return leaving;
    }

    // ----- Signals ------------------------------------------------------

    signal<view*> modal_pushed{};
    signal<view*> modal_popped{};

private:
    std::vector<view*> stack_{};
};

} // namespace mpapp::detail

#endif // MPAPP_DETAIL_MODAL_STACK_HPP
