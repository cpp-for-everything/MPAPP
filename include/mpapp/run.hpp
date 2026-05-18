// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0012-application-window-handler-abstraction.md
//
// `mpapp::run<App>(argc, argv)` — entry-point helper. Hides the
// platform-specific bootstrap goo (`MddBootstrap*` /
// `Application::Start` / `gtk_application_run` /
// `UIApplicationMain` / `NSApplicationMain` / `ANativeActivity_onCreate`)
// behind a single templated free function. User code's `main()` is one
// line:
//
//     int main(int argc, char** argv) {
//         return mpapp::run<my_app>(argc, argv);
//     }
//
// No public-API macro (per ADR-0002 / ADR-0009). The template parameter
// `App` must derive from `mpapp::application` and be default-constructible.
// The user app is constructed *inside* the platform handler's event
// loop, so its construction happens on the UI thread.

#ifndef MPAPP_RUN_HPP
#define MPAPP_RUN_HPP

#include <type_traits>

#include "application.hpp"
#include "native_handlers.hpp"

namespace mpapp {

template <class App>
int run(int argc, char** argv) {
    static_assert(std::is_base_of_v<application, App>,
                  "mpapp::run<App>: App must derive from mpapp::application");
    static_assert(std::is_default_constructible_v<App>,
                  "mpapp::run<App>: App must be default-constructible");

    application_handler<platform::current> handler{};
    return handler.template run_app<App>(argc, argv);
}

} // namespace mpapp

#endif // MPAPP_RUN_HPP
