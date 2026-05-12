// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::slider`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/slider_handler.hpp>
#include <mpapp/slider.hpp>

namespace {

using slider_mock = mpapp::slider_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("slider mock handler logs initial range", "[mock][slider]") {
    mpapp::slider s;
    slider_mock   h;

    s.minimum = 0.0;
    s.maximum = 100.0;
    s.value   = 50.0;

    h.map_minimum(s);
    h.map_maximum(s);
    h.map_value(s);

    REQUIRE(h.calls() == std::vector<std::string>{
                             "minimum=0",
                             "maximum=100",
                             "value=50",
                         });
}

TEST_CASE("slider mock handler fires once per real value change",
          "[mock][slider]") {
    mpapp::slider s;
    slider_mock   h;

    h.map_value(s);
    h.clear_calls();

    s.value = 0.25;
    s.value = 0.5;
    s.value = 0.5;  // no-op

    REQUIRE(h.calls() == std::vector<std::string>{
                             "value=0.25",
                             "value=0.5",
                         });
}

TEST_CASE("slider mock handler tracks min/max changes independently",
          "[mock][slider]") {
    mpapp::slider s;
    slider_mock   h;

    h.map_minimum(s);
    h.map_maximum(s);
    h.clear_calls();

    s.minimum = -10.0;
    s.maximum = 10.0;
    s.minimum = -10.0;  // no-op

    REQUIRE(h.calls() == std::vector<std::string>{
                             "minimum=-10",
                             "maximum=10",
                         });
}
