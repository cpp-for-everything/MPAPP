// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/WebView.md
//
// `mpapp::web_view` — native browser embed. The mock surface keeps the
// URL / HTML observables plus the back/forward/reload commands; the
// real per-platform handler binds these to WebView2 (Windows),
// WebKitGTK (Linux — see RFC-0001 § Linux licensing), Android WebView
// (Android), and WKWebView (macOS/iOS).

#ifndef MPAPP_WEB_VIEW_HPP
#define MPAPP_WEB_VIEW_HPP

#include <string>

#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class web_view_handler;

class web_view : public view {
public:
    web_view() = default;
    ~web_view() override = default;

    web_view(const web_view&)            = delete;
    web_view& operator=(const web_view&) = delete;
    web_view(web_view&&)                 = delete;
    web_view& operator=(web_view&&)      = delete;

    // ----- Surface ------------------------------------------------------

    Observable<std::string> url{""};
    Observable<std::string> html_source{""};
    Observable<bool>        is_loading{false};
    Observable<bool>        can_go_back{false};
    Observable<bool>        can_go_forward{false};

    // ----- Signals ------------------------------------------------------

    signal<const std::string&>           navigating{};   // emits URL just before nav
    signal<const std::string&, bool>     navigated{};    // emits (URL, success) after nav

    // ----- Commands -----------------------------------------------------

    // Mock implementations only flip the navigation state; real handlers
    // call into the native widget's history API.
    void load(const std::string& target_url) {
        url.set(target_url);
        is_loading.set(true);
        navigating.emit(target_url);
        // mock "completes" immediately
        is_loading.set(false);
        navigated.emit(target_url, true);
    }

    void load_html(const std::string& html) {
        html_source.set(html);
        is_loading.set(true);
        navigating.emit(std::string{"about:blank"});
        is_loading.set(false);
        navigated.emit(std::string{"about:blank"}, true);
    }

    void go_back() {
        if (!can_go_back.get()) return;
        // mock can't actually traverse history; just emits signals
        navigating.emit(url.get());
        navigated.emit(url.get(), true);
    }

    void go_forward() {
        if (!can_go_forward.get()) return;
        navigating.emit(url.get());
        navigated.emit(url.get(), true);
    }

    void reload() {
        is_loading.set(true);
        navigating.emit(url.get());
        is_loading.set(false);
        navigated.emit(url.get(), true);
    }

    // ----- Handler ------------------------------------------------------

    web_view_handler<platform::current>&       wv_handler() noexcept       { return *wv_handler_; }
    const web_view_handler<platform::current>& wv_handler() const noexcept { return *wv_handler_; }
    bool                                       has_wv_handler() const noexcept { return wv_handler_ != nullptr; }
    void                                       set_wv_handler(web_view_handler<platform::current>& h) noexcept { wv_handler_ = &h; }

private:
    web_view_handler<platform::current>* wv_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_WEB_VIEW_HPP
