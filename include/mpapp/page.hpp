// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Page.md
//
// `mpapp::page` — navigable content host. Mirrors MAUI's `ContentPage`:
// a title-carrying wrapper around a single content view. Useful as the
// `window.content` for a single-page app and as the leaf node for the
// future `navigation_page`/`tabbed_page` containers.
//
// Like `window`, `content` is a non-owning `view*` reference. The user
// owns the view's lifetime.

#ifndef MPAPP_PAGE_HPP
#define MPAPP_PAGE_HPP

#include <string>
#include <string_view>

#include "control.hpp"
#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform = platform::current>
class page_handler;

class page : public view {
public:
    page() = default;
    ~page() override = default;

    page(const page&)            = delete;
    page& operator=(const page&) = delete;
    page(page&&)                 = delete;
    page& operator=(page&&)      = delete;

    // ----- Properties ----------------------------------------------------
    Observable<std::string> title{""};
    Observable<view*>       content{nullptr};
    Observable<bool>        is_busy{false};

    // ----- Lifecycle signals ---------------------------------------------
    //
    // `shell` fires `navigated_to` on the page that becomes current_content
    // after a successful `go_to(uri)` (or typed go_to). It fires
    // `navigated_from` on the previously-current page just before swapping
    // it out. Both carry the relevant URI (the new route for navigated_to;
    // the previous route for navigated_from).
    //
    // Mirrors MAUI's `OnNavigatedTo` / `OnNavigatedFrom` overrides. Pages
    // typically subscribe in their constructor and use the URI to refresh
    // query-string-dependent state or persist data on leave.
    signal<const std::string& /*uri*/> navigated_to{};
    signal<const std::string& /*previous_uri*/> navigated_from{};

    // ----- Handler -------------------------------------------------------
    page_handler<platform::current>&       handler() noexcept       { return *page_handler_; }
    const page_handler<platform::current>& handler() const noexcept { return *page_handler_; }
    bool                                   has_handler() const noexcept { return page_handler_ != nullptr; }
    void                                   set_handler(page_handler<platform::current>& h) noexcept { page_handler_ = &h; }

private:
    // Distinct from `view::handler_` (the view_handler<>) — pages have a
    // dedicated page_handler<> that drives title / navigation chrome.
    page_handler<platform::current>* page_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_PAGE_HPP
