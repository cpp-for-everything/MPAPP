// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — AppKit application handler.
//
// `application_handler<platform::macos>` wraps `NSApplication`. The
// public surface mirrors the WinUI 3 / GTK4 handlers; AppKit-specific
// types live in the .mm implementation.

#ifndef MPAPP_HANDLERS_MACOS_APPLICATION_HANDLER_HPP
#define MPAPP_HANDLERS_MACOS_APPLICATION_HANDLER_HPP

#include "../../application.hpp"
#include "../../platform.hpp"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  if !TARGET_OS_IPHONE

namespace mpapp {

namespace detail {

struct appkit_application_launcher {
    application* (*construct)() = nullptr;
};

int appkit_run_app_impl(const appkit_application_launcher& launcher,
                        int argc, char** argv,
                        application*& out_app);

} // namespace detail

template <>
class application_handler<platform::macos> {
public:
    application_handler() = default;
    ~application_handler() = default;

    application_handler(const application_handler&)            = delete;
    application_handler& operator=(const application_handler&) = delete;
    application_handler(application_handler&&)                 = delete;
    application_handler& operator=(application_handler&&)      = delete;

    template <class App>
    int run_app(int argc, char** argv) {
        detail::appkit_application_launcher launcher{
            +[]() -> application* { return new App{}; }
        };
        application* raw_app = nullptr;
        const int rc = detail::appkit_run_app_impl(launcher, argc, argv, raw_app);
        delete raw_app;
        return rc;
    }
};

} // namespace mpapp

#  endif // !TARGET_OS_IPHONE
#endif // __APPLE__
#endif // MPAPP_HANDLERS_MACOS_APPLICATION_HANDLER_HPP
