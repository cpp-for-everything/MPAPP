// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Shell.md
//                  vault/20_ADRs/ADR-0014-page-navigation-stack.md
//
// `mpapp::shell` — top-level app shell with URI routing + tabs +
// flyout. Composes TabbedPage / NavigationPage / FlyoutPage at the
// page level. The mock surface focuses on the routing-and-navigation
// primitives:
//   - register_route(name) → record known routes
//   - go_to(uri)           → mock parser updates current_route +
//                           current_tab_index for routes of the
//                           form "//tab_name" or "//tab_name/leaf"
// Full Shell semantics (route templates with parameters, route guards,
// route lifecycle Aware interfaces) land with the URI routing ADR.

#ifndef MPAPP_SHELL_HPP
#define MPAPP_SHELL_HPP

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "observable.hpp"
#include "page.hpp"
#include "platform.hpp"
#include "route.hpp"
#include "signal.hpp"

namespace mpapp {

template <class Platform>
class shell_handler;

class shell : public page {
public:
    shell() = default;
    ~shell() override = default;

    shell(const shell&)            = delete;
    shell& operator=(const shell&) = delete;
    shell(shell&&)                 = delete;
    shell& operator=(shell&&)      = delete;

    // ----- Surface ------------------------------------------------------

    Observable<std::string>              current_route{"//"};
    Observable<std::vector<std::string>> tabs{};               // tab labels (top-level)
    Observable<int>                      current_tab_index{0};
    Observable<bool>                     is_flyout_open{false};
    Observable<page*>                    flyout_content{nullptr};
    // Main content area. Apps swap this on tab-selection / go_to events.
    Observable<page*>                    current_content{nullptr};

    // Known route names registered via register_route().
    Observable<std::vector<std::string>> registered_routes{};

    // ----- Signals ------------------------------------------------------

    signal<const std::string&> navigated{};      // emits the route after go_to() lands
    signal<bool>               flyout_toggled{}; // emits new is_flyout_open after flip

    // ----- Mutators -----------------------------------------------------

    void register_route(std::string_view name) {
        auto v = registered_routes.get();
        std::string s(name);
        if (std::find(v.begin(), v.end(), s) == v.end()) {
            v.push_back(std::move(s));
            registered_routes.set(std::move(v));
        }
    }

    void add_tab(std::string_view label) {
        auto v = tabs.get();
        v.emplace_back(label);
        tabs.set(std::move(v));
        if (current_tab_index.get() < 0 || current_tab_index.get() >= static_cast<int>(tabs.get().size())) {
            current_tab_index.set(0);
        }
    }

    // Parse the URI and update current_route / current_tab_index.
    // Recognized shapes:
    //   "//"               -> stay at current tab (no-op-ish, just sets route)
    //   "//tab_name"       -> first tab whose label matches becomes current
    //   "//tab_name/leaf"  -> same as above, leaf segment ignored for mock
    // Unrecognized routes still set current_route so observers see the change;
    // the tab index doesn't move.
    void go_to(std::string_view uri) {
        std::string u(uri);
        if (u.rfind("//", 0) == 0) {
            std::string_view tail = std::string_view{u}.substr(2);
            std::string_view tab_name = tail;
            // Cut at the first delimiter — either '/' (leaf path) or
            // '?' (query string). Whichever comes first wins.
            const auto slash = tab_name.find('/');
            const auto qmark = tab_name.find('?');
            const auto cut   = std::min(slash, qmark);
            if (cut != std::string_view::npos) tab_name = tab_name.substr(0, cut);
            if (!tab_name.empty()) {
                const auto& v = tabs.get();
                for (std::size_t i = 0; i < v.size(); ++i) {
                    if (v[i] == tab_name) {
                        current_tab_index.set(static_cast<int>(i));
                        break;
                    }
                }
            }
        }
        current_route.set(std::move(u));
        navigated.emit(current_route.get());
    }

    // Compile-time-checked navigation per
    // [[ADR-0016-shell-compile-time-routes]]. Path must be a route in
    // Table; arg count + types must match the route's params. Builds
    // the "//path?p1=v1&p2=v2" URI and delegates to the string-based
    // go_to() — so all the runtime side effects (current_route,
    // current_tab_index, navigated) work the same way regardless of
    // which entry point you took.
    //
    //   inline constexpr auto routes = mpapp::route_table{
    //       mpapp::route<"home/details", details_page,
    //                    mpapp::param<"id", int>>{},
    //   };
    //   shell.go_to<"home/details", &routes>(42);
    //
    // Table is taken by pointer so it can be a non-type template
    // parameter (C++20 — the pointed-to object must be a
    // constexpr-initialized variable with linkage; `inline constexpr`
    // covers it).
    template <detail::fixed_string Path, auto* Table, class... Args>
    void go_to(Args&&... args) {
        using table_type = std::remove_pointer_t<decltype(Table)>;
        static_assert(table_type::template has<Path>(),
                      "shell::go_to: route not in route_table");
        using route_t = typename table_type::template route_for<Path>;
        static_assert(std::tuple_size_v<typename route_t::params_t> == sizeof...(Args),
                      "shell::go_to: argument count doesn't match route's param count");
        // Per-arg type checking happens inside build_uri's append_args.
        std::string uri = table_type::template build_uri<Path>(std::forward<Args>(args)...);
        go_to(std::string_view{uri});
    }

    void open_flyout()  { set_flyout(true);  }
    void close_flyout() { set_flyout(false); }
    void toggle_flyout(){ set_flyout(!is_flyout_open.get()); }

    // ----- Handler ------------------------------------------------------

    shell_handler<platform::current>&       shell_handler_ref() noexcept       { return *handler_; }
    const shell_handler<platform::current>& shell_handler_ref() const noexcept { return *handler_; }
    bool                                    has_shell_handler() const noexcept { return handler_ != nullptr; }
    void                                    set_shell_handler(shell_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    void set_flyout(bool v) {
        if (is_flyout_open.get() == v) return;
        is_flyout_open.set(v);
        flyout_toggled.emit(v);
    }

    shell_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_SHELL_HPP
