// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_radio_button`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/radio_button_handler.hpp>
#include <mpapp/internal/basic_radio_button.hpp>

namespace {

using radio_button_mock = mpapp::internal::radio_button_handler<mpapp::platform::mock>;

} // namespace

TEST_CASE("radio_button mock handler logs initial checked and group",
          "[mock][radio_button]") {
    mpapp::internal::basic_radio_button r;
    radio_button_mock   h;

    r.group_name = "size";

    h.map_is_checked(r);
    h.map_group_name(r);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
                             "is_checked=false",
                             "group_name=size",
                         });
}

TEST_CASE("radio_button mock handler fires once per checked change",
          "[mock][radio_button]") {
    mpapp::internal::basic_radio_button r;
    radio_button_mock   h;

    h.map_is_checked(r);
    h.clear_calls();

    r.is_checked = true;
    r.is_checked = true;  // no-op
    r.is_checked = false;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
                             "is_checked=true",
                             "is_checked=false",
                         });
}

TEST_CASE("radio_button mock handler tracks group reassignment",
          "[mock][radio_button]") {
    mpapp::internal::basic_radio_button r;
    radio_button_mock   h;

    h.map_group_name(r);
    h.clear_calls();

    r.group_name = "size";
    r.group_name = "size";  // no-op
    r.group_name = "color";

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
                             "group_name=size",
                             "group_name=color",
                         });
}
