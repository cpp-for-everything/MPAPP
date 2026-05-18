// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — WinUI 3 application handler.
//
// `application_handler<platform::windows>` — hides the WinUI 3
// bootstrap goo (`MddBootstrapInitialize2`,
// `WindowsAppRuntime_EnsureIsLoaded`, `winrt::init_apartment`,
// `mux::Application::Start`, the `mux::ApplicationT<App>` subclass)
// behind `run_app<App>(argc, argv)`. User code's `main` is one line:
//
//     int main(int argc, char** argv) {
//         return mpapp::run<my_app>(argc, argv);
//     }
//
// The user's `mpapp::application` subclass is constructed *inside* the
// internal `mux::ApplicationT<App>::OnLaunched`, which is the only safe
// place to touch WinUI types (per the T-0003 RPC_E_WRONG_THREAD notes).

#ifndef MPAPP_HANDLERS_WINDOWS_APPLICATION_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_APPLICATION_HANDLER_HPP

#include "../../application.hpp"
#include "../../platform.hpp"

#if defined(_WIN32)

#include <memory>

namespace mpapp {

namespace detail {

// Type-erased launcher: holds the construct-and-on_launch callback
// the internal `mux::ApplicationT` subclass invokes when WinUI is
// ready. Defined in the .cpp so this header doesn't drag in WinRT
// projection headers — keeps user TUs lean.
struct application_launcher {
    // Returns the heap-allocated user app instance. The handler owns it.
    application* (*construct)() = nullptr;
};

// Hidden from user code: drives MddBootstrap + Application::Start.
// Implemented in src/handlers/windows/application_handler.cpp.
int run_app_impl(const application_launcher& launcher,
                 int argc, char** argv,
                 application*& out_app);

} // namespace detail

template <>
class application_handler<platform::windows> {
public:
    application_handler() = default;
    ~application_handler() = default;

    application_handler(const application_handler&)            = delete;
    application_handler& operator=(const application_handler&) = delete;
    application_handler(application_handler&&)                 = delete;
    application_handler& operator=(application_handler&&)      = delete;

    // Entry-point implementation called by `mpapp::run<App>`. Constructs
    // the user App inside WinUI's UI thread and blocks until the
    // platform event loop exits.
    template <class App>
    int run_app(int argc, char** argv) {
        detail::application_launcher launcher{
            +[]() -> application* { return new App{}; }
        };
        application* raw_app = nullptr;
        const int rc = detail::run_app_impl(launcher, argc, argv, raw_app);
        delete raw_app;
        return rc;
    }
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_APPLICATION_HANDLER_HPP
