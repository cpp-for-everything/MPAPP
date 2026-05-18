// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 application handler.
//
// `application_handler<platform::linux_>` wraps a `GtkApplication`. The
// public surface is identical to the WinUI 3 handler: `run_app<App>`
// constructs the user's `mpapp::application` subclass *inside* the
// GtkApplication "activate" signal callback, so all GTK4 widgets the
// user's `on_launch` constructs are created on the GTK main thread.
//
// All GTK4 / GLib types are hidden from this header — the implementation
// .cpp owns those.

#ifndef MPAPP_HANDLERS_LINUX_APPLICATION_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_APPLICATION_HANDLER_HPP

#include "../../application.hpp"
#include "../../platform.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

namespace detail {

struct gtk_application_launcher {
    application* (*construct)() = nullptr;
};

int gtk_run_app_impl(const gtk_application_launcher& launcher,
                     int argc, char** argv,
                     application*& out_app);

} // namespace detail

template <>
class application_handler<platform::linux_> {
public:
    application_handler() = default;
    ~application_handler() = default;

    application_handler(const application_handler&)            = delete;
    application_handler& operator=(const application_handler&) = delete;
    application_handler(application_handler&&)                 = delete;
    application_handler& operator=(application_handler&&)      = delete;

    template <class App>
    int run_app(int argc, char** argv) {
        detail::gtk_application_launcher launcher{
            +[]() -> application* { return new App{}; }
        };
        application* raw_app = nullptr;
        const int rc = detail::gtk_run_app_impl(launcher, argc, argv, raw_app);
        delete raw_app;
        return rc;
    }
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_APPLICATION_HANDLER_HPP
