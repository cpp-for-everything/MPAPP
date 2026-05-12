// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::stepper`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/stepper_handler.hpp>
#include <mpapp/stepper.hpp>

namespace {

using stepper_mock = mpapp::stepper_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("stepper mock handler logs initial value and interval",
          "[mock][stepper]") {
    mpapp::stepper s;
    stepper_mock   h;

    s.interval = 0.5;
    s.value    = 2.0;

    h.map_value(s);
    h.map_interval(s);

    REQUIRE(h.calls() == std::vector<std::string>{
                             "value=2",
                             "interval=0.5",
                         });
}

TEST_CASE("stepper mock handler fires once per real value change",
          "[mock][stepper]") {
    mpapp::stepper s;
    stepper_mock   h;

    h.map_value(s);
    h.clear_calls();

    s.value = 1.0;
    s.value = 2.0;
    s.value = 2.0;  // no-op

    REQUIRE(h.calls() == std::vector<std::string>{
                             "value=1",
                             "value=2",
                         });
}

TEST_CASE("stepper mock handler tracks interval changes",
          "[mock][stepper]") {
    mpapp::stepper s;
    stepper_mock   h;

    h.map_interval(s);
    h.clear_calls();

    s.interval = 0.25;
    s.interval = 0.25;  // no-op
    s.interval = 1.0;

    REQUIRE(h.calls() == std::vector<std::string>{
                             "interval=0.25",
                             "interval=1",
                         });
}
