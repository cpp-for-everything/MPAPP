// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/NavigationPage.md
//                  vault/20_ADRs/ADR-0014-page-navigation-stack.md
//
// `mpapp::navigation_page` — stack-based navigation host. Owns a
// `detail::page_stack`; the public surface exposes:
//   - Read-only stack state Observables (current_page, root_page, depth).
//   - Sync mutators (push, pop, pop_to_root, insert_page_before, remove_page).
//     Per ADR-0014, sync is the primitive; the planned async variants are
//     thin executor-bound wrappers and will land alongside the executor work.
//   - Attached-property setters (set_has_back_button, set_title, etc.) that
//     write into the page_stack's attached store keyed on the child page.
//
// The mock-first surface keeps the API narrow. Bar styling Observables
// (bar_background_color, bar_text_color, etc.) ship as no-op slots so the
// XAML surface compiles; their handlers attach in the per-platform real
// landings.

#ifndef MPAPP_NAVIGATION_PAGE_HPP
#define MPAPP_NAVIGATION_PAGE_HPP

#include <cstddef>
#include <string>
#include <string_view>

#include "detail/page_stack.hpp"
#include "executor.hpp"
#include "observable.hpp"
#include "page.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform>
class navigation_page_handler;

class navigation_page : public page {
public:
    navigation_page() = default;
    explicit navigation_page(page* root) {
        if (root != nullptr) stack_.push(root);
        sync_observables();
    }
    ~navigation_page() override = default;

    navigation_page(const navigation_page&)            = delete;
    navigation_page& operator=(const navigation_page&) = delete;
    navigation_page(navigation_page&&)                 = delete;
    navigation_page& operator=(navigation_page&&)      = delete;

    // ----- Read-only stack state ---------------------------------------

    Observable<page*>       current_page{nullptr};
    Observable<page*>       root_page{nullptr};
    Observable<std::size_t> stack_depth{0};

    // ----- Sync mutators (ADR-0014 primitive) --------------------------

    void push(page* p) {
        stack_.push(p);
        sync_observables();
    }

    page* pop() {
        view* v = stack_.pop();
        sync_observables();
        return static_cast<page*>(v);
    }

    void pop_to_root() {
        stack_.pop_to_root();
        sync_observables();
    }

    void insert_page_before(page* before, page* p) {
        stack_.insert_before(before, p);
        sync_observables();
    }

    void remove_page(page* p) {
        stack_.remove(p);
        sync_observables();
    }

    // ----- Async wrappers (ADR-0014 §Decision: sync as primitive) ------
    //
    // The async variants are thin coroutine wrappers around the sync
    // mutators above. They suspend-and-resume immediately in the mock
    // build; per-platform real handlers can override resumption to wait
    // for transition animations once ADR-0019's native dispatchers wire
    // through.

    [[nodiscard]] task<void> push_async(page* p) {
        push(p);
        co_return;
    }

    [[nodiscard]] task<page*> pop_async() {
        page* popped = pop();
        co_return popped;
    }

    [[nodiscard]] task<void> pop_to_root_async() {
        pop_to_root();
        co_return;
    }

    // ----- Attached-property setters (child sets, host reads) ----------

    // Each setter writes into the underlying page_stack store keyed on
    // the child page. The navigation host reads these when it renders the
    // bar chrome for the current page.

    static constexpr detail::attached_prop_key<bool>        has_back_button_key    {"has_back_button"};
    static constexpr detail::attached_prop_key<bool>        has_navigation_bar_key {"has_navigation_bar"};
    static constexpr detail::attached_prop_key<std::string> back_button_title_key  {"back_button_title"};

    void set_has_back_button(page& p, bool v) {
        stack_.set_attached(p, has_back_button_key, v);
    }
    bool get_has_back_button(const page& p) const {
        return stack_.get_attached(p, has_back_button_key, true);
    }

    void set_has_navigation_bar(page& p, bool v) {
        stack_.set_attached(p, has_navigation_bar_key, v);
    }
    bool get_has_navigation_bar(const page& p) const {
        return stack_.get_attached(p, has_navigation_bar_key, true);
    }

    void set_back_button_title(page& p, std::string_view t) {
        stack_.set_attached(p, back_button_title_key, std::string(t));
    }
    std::string get_back_button_title(const page& p) const {
        return stack_.get_attached(p, back_button_title_key, std::string{});
    }

    // ----- Engine access for handlers ----------------------------------

    detail::page_stack&       stack() noexcept       { return stack_; }
    const detail::page_stack& stack() const noexcept { return stack_; }

    // ----- Handler -----------------------------------------------------

    navigation_page_handler<platform::current>&       np_handler() noexcept       { return *nav_handler_; }
    const navigation_page_handler<platform::current>& np_handler() const noexcept { return *nav_handler_; }
    bool                                              has_np_handler() const noexcept { return nav_handler_ != nullptr; }
    void                                              set_np_handler(navigation_page_handler<platform::current>& h) noexcept { nav_handler_ = &h; }

private:
    void sync_observables() {
        page* root = static_cast<page*>(stack_.root());
        page* top  = static_cast<page*>(stack_.top());
        if (root_page.get()    != root) root_page.set(root);
        if (current_page.get() != top)  current_page.set(top);
        if (stack_depth.get()  != stack_.depth()) stack_depth.set(stack_.depth());
    }

    detail::page_stack                              stack_{};
    navigation_page_handler<platform::current>*     nav_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_NAVIGATION_PAGE_HPP
