// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android application handler.
//
// Android's lifecycle is inverted: the OS owns main(), and the user's
// Activity calls into native code. `application_handler<platform::android>`
// therefore does NOT spin its own event loop — it just constructs the
// user's App and calls on_launch when `run_app<App>` is invoked from
// the Activity's native init method.
//
// Wire-up: in the user's Android app, `MainActivity.onCreate` calls a
// `native void launchMpapp()` method that bridges to
// `mpapp::run<MyApp>(0, nullptr)`. The Activity, JavaVM, and main
// looper are already running by then.

#ifndef MPAPP_HANDLERS_ANDROID_APPLICATION_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_APPLICATION_HANDLER_HPP

#include "../../application.hpp"
#include "../../platform.hpp"

#if defined(__ANDROID__)

#include "jni_bridge.hpp"

namespace mpapp {

template <>
class application_handler<platform::android> {
public:
    application_handler() = default;
    ~application_handler() = default;

    application_handler(const application_handler&)            = delete;
    application_handler& operator=(const application_handler&) = delete;
    application_handler(application_handler&&)                 = delete;
    application_handler& operator=(application_handler&&)      = delete;

    template <class App>
    int run_app(int /*argc*/, char** /*argv*/) {
        // Static so it outlives the handler — Android may call back
        // into native code from Java for lifecycle events well after
        // run_app returns.
        static App* user_app = nullptr;
        if (user_app == nullptr) {
            user_app = new App{};
            user_app->on_launch();
        }
        return 0;
    }
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_APPLICATION_HANDLER_HPP
