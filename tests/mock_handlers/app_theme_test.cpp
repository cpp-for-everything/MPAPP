// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for `mpapp::application::requested_theme`
// (MAUI Application.RequestedTheme).

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/application.hpp>
#include <mpapp/essentials/app_info.hpp>

using namespace mpapp;

namespace {

class test_app : public application {
public:
    void on_launch() override {}
};

} // namespace

TEST_CASE("application defaults requested_theme to unspecified",
          "[mock][app_theme]") {
    test_app app;
    CHECK(app.requested_theme() == app_theme::unspecified);
}

TEST_CASE("set_requested_theme updates the stored theme",
          "[mock][app_theme]") {
    test_app app;

    app.set_requested_theme(app_theme::dark);
    CHECK(app.requested_theme() == app_theme::dark);

    app.set_requested_theme(app_theme::light);
    CHECK(app.requested_theme() == app_theme::light);
}

TEST_CASE("set_requested_theme emits requested_theme_changed on change",
          "[mock][app_theme]") {
    test_app app;

    std::vector<app_theme> log;
    auto on_change = [&](app_theme t) { log.push_back(t); };
    mpapp::signal<app_theme>::slot_type slot;
    app.requested_theme_changed.subscribe(slot, on_change);

    app.set_requested_theme(app_theme::dark);
    app.set_requested_theme(app_theme::light);

    REQUIRE(log.size() == 2);
    CHECK(log[0] == app_theme::dark);
    CHECK(log[1] == app_theme::light);
}

TEST_CASE("set_requested_theme to the same value is a no-op (no signal)",
          "[mock][app_theme]") {
    test_app app;
    app.set_requested_theme(app_theme::dark);

    int count = 0;
    auto on_change = [&](app_theme) { ++count; };
    mpapp::signal<app_theme>::slot_type slot;
    app.requested_theme_changed.subscribe(slot, on_change);

    app.set_requested_theme(app_theme::dark);   // same value -> suppressed
    CHECK(count == 0);
    CHECK(app.requested_theme() == app_theme::dark);

    app.set_requested_theme(app_theme::light);  // different -> fires once
    CHECK(count == 1);
}

TEST_CASE("setting the default theme from unspecified still emits once",
          "[mock][app_theme]") {
    test_app app;

    int count = 0;
    auto on_change = [&](app_theme) { ++count; };
    mpapp::signal<app_theme>::slot_type slot;
    app.requested_theme_changed.subscribe(slot, on_change);

    // unspecified -> unspecified is a no-op.
    app.set_requested_theme(app_theme::unspecified);
    CHECK(count == 0);

    // unspecified -> light fires.
    app.set_requested_theme(app_theme::light);
    CHECK(count == 1);
}
