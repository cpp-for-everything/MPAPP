// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit application handler.

#ifndef MPAPP_HANDLERS_IOS_APPLICATION_HANDLER_HPP
#define MPAPP_HANDLERS_IOS_APPLICATION_HANDLER_HPP

#include "../../application.hpp"
#include "../../platform.hpp"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE

namespace mpapp {

namespace detail {

struct uikit_application_launcher {
    application* (*construct)() = nullptr;
};

int uikit_run_app_impl(const uikit_application_launcher& launcher,
                       int argc, char** argv,
                       application*& out_app);

} // namespace detail

template <>
class application_handler<platform::ios> {
public:
    application_handler() = default;
    ~application_handler() = default;

    application_handler(const application_handler&)            = delete;
    application_handler& operator=(const application_handler&) = delete;
    application_handler(application_handler&&)                 = delete;
    application_handler& operator=(application_handler&&)      = delete;

    template <class App>
    int run_app(int argc, char** argv) {
        detail::uikit_application_launcher launcher{
            +[]() -> application* { return new App{}; }
        };
        application* raw_app = nullptr;
        const int rc = detail::uikit_run_app_impl(launcher, argc, argv, raw_app);
        delete raw_app;
        return rc;
    }
};

} // namespace mpapp

#  endif // TARGET_OS_IPHONE
#endif // __APPLE__
#endif // MPAPP_HANDLERS_IOS_APPLICATION_HANDLER_HPP
