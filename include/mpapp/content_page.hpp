// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ContentPage.md
//
// `mpapp::content_page` — the simplest concrete `mpapp::page` subclass.
// Hosts a single child view, exposes a page-level `title`, and applies
// platform padding around the content.
//
// Mirrors MAUI's `ContentPage` (the workhorse leaf for 99% of app
// screens; the default content for NavigationPage / TabbedPage / Shell).
// Mock surface (P2) — keeps `hide_soft_input_on_tapped` /
// `safe_area_edges` off the C++ class until the safe-area types land in
// P3; tests + handlers only touch content / title / padding.
//
// The `content` Observable differs from base `page::content` (which is
// `Observable<view*>`): on `content_page` it is `Observable<shared_ptr<view>>`
// so the handlers can share the single-child wrap pattern used by
// `content_view` / `border` / `scroll_view`. The shadowing is
// intentional — accessing `cp.content` through a `content_page&` resolves
// to the shared_ptr Observable; accessing through a `page&` falls back
// to the base's view* surface. Real platforms only ever set the derived
// `content`.

#ifndef MPAPP_CONTENT_PAGE_HPP
#define MPAPP_CONTENT_PAGE_HPP

#include <memory>
#include <string>

#include "layout.hpp"     // for `thickness`
#include "observable.hpp"
#include "page.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class content_page_handler;

class content_page : public page {
public:
    content_page() = default;
    ~content_page() override = default;

    content_page(const content_page&)            = delete;
    content_page& operator=(const content_page&) = delete;
    content_page(content_page&&)                 = delete;
    content_page& operator=(content_page&&)      = delete;

    // ----- Properties ----------------------------------------------------
    // Hosted view (shared_ptr-typed; shadows base `page::content`).
    Observable<std::shared_ptr<view>>   content{};
    // Page padding. Base `page` does not yet expose padding; declared
    // here per the Worker prompt.
    Observable<thickness>               padding{};
    // `title` lives on `page` already and is reused as-is.

    // ----- Handler -------------------------------------------------------
    content_page_handler<platform::current>&       handler() noexcept       { return *content_page_handler_; }
    const content_page_handler<platform::current>& handler() const noexcept { return *content_page_handler_; }
    bool                                           has_handler() const noexcept { return content_page_handler_ != nullptr; }
    void                                           set_handler(content_page_handler<platform::current>& h) noexcept { content_page_handler_ = &h; }

private:
    // Distinct from base `page::page_handler_` and `view::handler_`.
    content_page_handler<platform::current>* content_page_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_CONTENT_PAGE_HPP
