// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler smoke test for `mpapp::internal::basic_menu_flyout_separator`
// (M-04b). The separator has no observable properties - this test
// exists to satisfy the per-widget test-file glob and to verify the
// handler can bind without throwing.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/menu_flyout_separator_handler.hpp>
#include <mpapp/internal/basic_menu_flyout_separator.hpp>

using namespace mpapp;

TEST_CASE("menu_flyout_separator mock handler records the bare bind event",
          "[mock][menu_flyout_separator]") {
    internal::basic_menu_flyout_separator s;
    internal::menu_flyout_separator_handler<platform::mock> h;

    h.map_bind(s);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "separator");
    CHECK(h.calls()[0].has_value     == false);
}
