// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_flyout_view`.

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include <mpapp/internal/basic_flyout_view.hpp>
#include <mpapp/handlers/mock/flyout_view_handler.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class plain_view : public view {};

} // namespace

TEST_CASE("flyout_view mock records initial values on bind",
          "[mock][flyout_view]") {
    internal::basic_flyout_view fv;
    internal::flyout_view_handler<platform::mock> h;

    h.map_flyout(fv);
    h.map_detail(fv);
    h.map_is_presented(fv);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "flyout.present");
    CHECK(h.calls()[0].value_repr    == "false");
    CHECK(h.calls()[1].property_name == "detail.present");
    CHECK(h.calls()[1].value_repr    == "false");
    CHECK(h.calls()[2].property_name == "is_presented");
    CHECK(h.calls()[2].value_repr    == "false");
}

TEST_CASE("flyout_view mock tracks pane assignments and drawer toggles",
          "[mock][flyout_view]") {
    internal::basic_flyout_view fv;
    internal::flyout_view_handler<platform::mock> h;

    h.map_flyout(fv);
    h.map_detail(fv);
    h.map_is_presented(fv);
    h.clear_calls();

    // Assign the two panes - each fires a presence-flip record.
    fv.flyout = std::make_shared<plain_view>();
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "flyout.present");
    CHECK(h.calls()[0].value_repr    == "true");

    fv.detail = std::make_shared<plain_view>();
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].property_name == "detail.present");
    CHECK(h.calls()[1].value_repr    == "true");

    // Toggle the drawer open, idempotently re-toggle, then close.
    fv.is_presented = true;
    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[2].property_name == "is_presented");
    CHECK(h.calls()[2].value_repr    == "true");

    fv.is_presented = true;             // idempotent - no extra row
    REQUIRE(h.calls().size() == 3);

    fv.is_presented = false;
    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[3].property_name == "is_presented");
    CHECK(h.calls()[3].value_repr    == "false");
}
