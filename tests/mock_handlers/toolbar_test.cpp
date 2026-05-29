// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_toolbar`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/toolbar_handler.hpp>
#include <mpapp/internal/basic_toolbar.hpp>

using namespace mpapp;

TEST_CASE("toolbar mock records initial values on bind",
          "[mock][toolbar]") {
    internal::basic_toolbar t;
    internal::toolbar_handler<platform::mock> h;

    h.map_items(t);
    h.map_title(t);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "items.count");
    CHECK(h.calls()[0].value_repr    == "0");
    CHECK(h.calls()[1].property_name == "title");
    CHECK(h.calls()[1].value_repr    == "");
}

TEST_CASE("toolbar records items.count when collection changes",
          "[mock][toolbar]") {
    internal::basic_toolbar t;
    internal::toolbar_handler<platform::mock> h;

    h.map_items(t);
    h.clear_calls();

    t.items = std::vector<toolbar_item>{
        {"Compose", ""},
        {"Refresh", ""},
        {"Settings", ""},
    };
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "items.count");
    CHECK(h.calls()[0].value_repr    == "3");

    // Idempotent — equal vector should not re-record.
    t.items = std::vector<toolbar_item>{
        {"Compose", ""},
        {"Refresh", ""},
        {"Settings", ""},
    };
    REQUIRE(h.calls().size() == 1);

    t.items = std::vector<toolbar_item>{};
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "0");
}

TEST_CASE("toolbar records title changes",
          "[mock][toolbar]") {
    internal::basic_toolbar t;
    internal::toolbar_handler<platform::mock> h;

    h.map_title(t);
    h.clear_calls();

    t.title = "Inbox";
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "title");
    CHECK(h.calls()[0].value_repr    == "Inbox");

    t.title = "Inbox";               // idempotent
    REQUIRE(h.calls().size() == 1);

    t.title = "Drafts";
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "Drafts");
}
