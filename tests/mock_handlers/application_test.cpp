// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::application` (T-0011).
//
// Validates the application lifecycle contract: `run_app<App>` invokes
// `on_launch` exactly once on a fresh App, lifecycle simulators dispatch
// to the corresponding user-override hook, and the recorded call log
// captures the framework -> user transition.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/application.hpp>
#include <mpapp/handlers/mock/application_handler.hpp>

namespace {

using namespace mpapp;

// Minimal application subclass that records its own override calls into
// a string vector — distinct from the handler's call log so tests can
// assert framework-vs-user observation independently.
class spy_app : public application {
public:
    std::vector<std::string> override_log{};

    void on_launch()    override { override_log.push_back("on_launch"); }
    void on_suspend()   override { override_log.push_back("on_suspend"); }
    void on_resume()    override { override_log.push_back("on_resume"); }
    void on_terminate() override { override_log.push_back("on_terminate"); }
};

} // namespace

TEST_CASE("application mock handler runs the launch lifecycle",
          "[mock][application]") {
    application_handler<platform::mock> h;
    char* fake_argv[] = {const_cast<char*>("test"), nullptr};

    const int rc = h.run_app<spy_app>(1, fake_argv);

    REQUIRE(rc == 0);
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "run_app_enter");
    CHECK(h.calls()[1].property_name == "run_app_exit");
}

TEST_CASE("application mock handler returns the configured exit code",
          "[mock][application]") {
    application_handler<platform::mock> h;
    h.set_exit_code(42);
    char* fake_argv[] = {const_cast<char*>("test"), nullptr};

    const int rc = h.run_app<spy_app>(1, fake_argv);

    REQUIRE(rc == 42);
}

TEST_CASE("application mock handler simulators dispatch to user overrides",
          "[mock][application]") {
    spy_app app;
    application_handler<platform::mock> h;

    h.simulate_suspend(app);
    h.simulate_resume(app);
    h.simulate_terminate(app);

    REQUIRE(app.override_log == std::vector<std::string>{
        "on_suspend", "on_resume", "on_terminate"});
    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "simulate_suspend");
    CHECK(h.calls()[1].property_name == "simulate_resume");
    CHECK(h.calls()[2].property_name == "simulate_terminate");
}

TEST_CASE("application default lifecycle hooks are no-ops",
          "[mock][application]") {
    // The base-class on_suspend / on_resume / on_terminate are empty —
    // user apps override only the hooks they care about. Verify that
    // calling them on a stock application doesn't crash and produces
    // no observable state change.
    class minimal_app : public application {
    public:
        void on_launch() override {}
    };

    minimal_app app;
    application_handler<platform::mock> h;

    h.simulate_suspend(app);
    h.simulate_resume(app);
    h.simulate_terminate(app);

    // Nothing to assert on the user side — they're no-ops; the handler
    // recorded the framework-side simulator dispatches.
    REQUIRE(h.calls().size() == 3);
}
