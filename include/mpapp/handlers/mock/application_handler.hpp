// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Application.md
//
// `application_handler<platform::mock>` — records lifecycle invocations
// (launch, suspend, resume, terminate) for unit tests. Provides a
// `run_app<App>` template that drives the lifecycle without a real
// event loop so tests can run synchronously.

#ifndef MPAPP_HANDLERS_MOCK_APPLICATION_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_APPLICATION_HANDLER_HPP

#include "../../application.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class application_handler<platform::mock> : public mock_handler_base {
public:
    application_handler() = default;
    ~application_handler() = default;

    application_handler(const application_handler&)            = delete;
    application_handler& operator=(const application_handler&) = delete;
    application_handler(application_handler&&)                 = delete;
    application_handler& operator=(application_handler&&)      = delete;

    // Synchronous "event loop" used by tests. Constructs the user App,
    // calls `on_launch` (and optionally the other lifecycle hooks via
    // `simulate_*` helpers), records each call, returns the configured
    // exit code. Does *not* go through `application::set_handler` —
    // that API is typed on `platform::current` and would not accept
    // `application_handler<platform::mock>` on a host build where
    // current != mock. Tests that need `has_handler() == true` inside
    // `on_launch` should drive the lifecycle directly against a stack
    // App instance (see application_test.cpp).
    template <class App>
    int run_app(int /*argc*/, char** /*argv*/) {
        record("run_app_enter");
        App user_app{};
        user_app.on_launch();
        record("run_app_exit");
        return exit_code_;
    }

    // Lifecycle simulators — tests call these to drive the suspend /
    // resume / terminate paths a real platform handler would emit.
    void simulate_suspend(application& app) {
        record("simulate_suspend");
        app.on_suspend();
    }
    void simulate_resume(application& app) {
        record("simulate_resume");
        app.on_resume();
    }
    void simulate_terminate(application& app) {
        record("simulate_terminate");
        app.on_terminate();
    }

    void set_exit_code(int code) noexcept { exit_code_ = code; }

private:
    int exit_code_ = 0;
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_APPLICATION_HANDLER_HPP
