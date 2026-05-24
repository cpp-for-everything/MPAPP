// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_menu_flyout_item` (M-04b).

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/menu_flyout_item_handler.hpp>
#include <mpapp/menu_flyout_item.hpp>

using namespace mpapp;

TEST_CASE("menu_flyout_item mock records initial property values on bind",
          "[mock][menu_flyout_item]") {
    internal::basic_menu_flyout_item i;
    menu_flyout_item_handler<platform::mock> h;

    h.map_text(i);
    h.map_is_enabled(i);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "text");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "is_enabled");
    CHECK(h.calls()[1].value_repr    == "true");
}

TEST_CASE("menu_flyout_item mock tracks text + is_enabled changes",
          "[mock][menu_flyout_item]") {
    internal::basic_menu_flyout_item i;
    menu_flyout_item_handler<platform::mock> h;

    h.map_text(i);
    h.map_is_enabled(i);
    h.clear_calls();

    i.text       = "Cut";
    i.text       = "Cut";   // idempotent — no extra row
    i.is_enabled = false;
    i.text       = "Copy";

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "text");
    CHECK(h.calls()[0].value_repr    == "Cut");
    CHECK(h.calls()[1].property_name == "is_enabled");
    CHECK(h.calls()[1].value_repr    == "false");
    CHECK(h.calls()[2].property_name == "text");
    CHECK(h.calls()[2].value_repr    == "Copy");
}

TEST_CASE("menu_flyout_item clicked signal fires for subscribers",
          "[mock][menu_flyout_item]") {
    internal::basic_menu_flyout_item i;

    int fire_count = 0;
    struct counter_t { int* n; void operator()() const { ++(*n); } } counter{&fire_count};
    signal_slot<> slot;
    i.clicked.subscribe(slot, counter);

    // The mock handler doesn't drive `clicked`; that's a real-handler
    // concern. We verify the signal works end-to-end via direct emit
    // — this is what a real platform handler would do after the
    // native control's click callback fires.
    i.clicked.emit();
    i.clicked.emit();
    CHECK(fire_count == 2);
}
