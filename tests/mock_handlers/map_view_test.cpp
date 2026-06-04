// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MapView.md
//
// Mock-handler tests for `mpapp::internal::basic_map_view` (ADR-0008).
// Covers every Observable property, the pin collection API, and both
// signals (pin_clicked, map_clicked).

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_map_view.hpp>
#include <mpapp/handlers/mock/map_view_handler.hpp>

namespace {

using map_mock = mpapp::internal::map_view_handler<mpapp::platform::mock>;

} // namespace

// ---------------------------------------------------------------------------
// map_kind
// ---------------------------------------------------------------------------

TEST_CASE("map_view mock handler records initial kind on map", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_kind(m);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"kind=street"});
}

TEST_CASE("map_view mock handler records kind change", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_kind(m);
    h.clear_calls();

    m.kind = mpapp::map_kind::satellite;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"kind=satellite"});

    m.kind = mpapp::map_kind::hybrid;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"kind=satellite", "kind=hybrid"});
}

TEST_CASE("map_view mock handler ignores same kind write", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    m.kind = mpapp::map_kind::street;
    h.map_kind(m);
    h.clear_calls();

    m.kind = mpapp::map_kind::street;

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// center
// ---------------------------------------------------------------------------

TEST_CASE("map_view mock handler records initial center on map", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_center(m);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"center=geo(0,0)"});
}

TEST_CASE("map_view mock handler records center change", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_center(m);
    h.clear_calls();

    m.center = mpapp::geo_point{51.5074, -0.1278};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "center");
}

TEST_CASE("map_view mock handler ignores same center write", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_center(m);
    h.clear_calls();

    m.center = mpapp::geo_point{0.0, 0.0};
    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// zoom
// ---------------------------------------------------------------------------

TEST_CASE("map_view mock handler records initial zoom on map", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_zoom(m);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"zoom=1"});
}

TEST_CASE("map_view mock handler records zoom change", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_zoom(m);
    h.clear_calls();

    m.zoom = 10.0;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"zoom=10"});
}

TEST_CASE("map_view mock handler ignores same zoom write", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_zoom(m);
    h.clear_calls();

    m.zoom = 1.0;
    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// is_showing_user
// ---------------------------------------------------------------------------

TEST_CASE("map_view mock handler records initial is_showing_user", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_is_showing_user(m);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_showing_user=false"});
}

TEST_CASE("map_view mock handler records is_showing_user change", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_is_showing_user(m);
    h.clear_calls();

    m.is_showing_user = true;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_showing_user=true"});
}

// ---------------------------------------------------------------------------
// is_traffic_enabled
// ---------------------------------------------------------------------------

TEST_CASE("map_view mock handler records initial is_traffic_enabled", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_is_traffic_enabled(m);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_traffic_enabled=false"});
}

TEST_CASE("map_view mock handler records is_traffic_enabled change", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_is_traffic_enabled(m);
    h.clear_calls();

    m.is_traffic_enabled = true;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_traffic_enabled=true"});

    m.is_traffic_enabled = true;
    REQUIRE(h.calls().size() == 1);
}

// ---------------------------------------------------------------------------
// is_scroll_enabled
// ---------------------------------------------------------------------------

TEST_CASE("map_view mock handler records initial is_scroll_enabled", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_is_scroll_enabled(m);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_scroll_enabled=true"});
}

TEST_CASE("map_view mock handler records is_scroll_enabled change", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_is_scroll_enabled(m);
    h.clear_calls();

    m.is_scroll_enabled = false;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_scroll_enabled=false"});
}

// ---------------------------------------------------------------------------
// is_zoom_enabled
// ---------------------------------------------------------------------------

TEST_CASE("map_view mock handler records initial is_zoom_enabled", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_is_zoom_enabled(m);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_zoom_enabled=true"});
}

TEST_CASE("map_view mock handler records is_zoom_enabled change", "[mock][map_view]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_is_zoom_enabled(m);
    h.clear_calls();

    m.is_zoom_enabled = false;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_zoom_enabled=false"});
}

// ---------------------------------------------------------------------------
// pin collection
// ---------------------------------------------------------------------------

TEST_CASE("map_view starts with no pins", "[mock][map_view][pins]") {
    mpapp::internal::basic_map_view m;

    REQUIRE(m.pin_count() == 0);
}

TEST_CASE("map_view add_pin increases count", "[mock][map_view][pins]") {
    mpapp::internal::basic_map_view m;

    m.add_pin(mpapp::map_pin{"Buckingham Palace", "London SW1A 1AA", {51.5014, -0.1419}});

    REQUIRE(m.pin_count() == 1);
}

TEST_CASE("map_view pin_at returns correct pin", "[mock][map_view][pins]") {
    mpapp::internal::basic_map_view m;

    mpapp::map_pin expected{"Big Ben", "Westminster, London", {51.5007, -0.1246}};
    m.add_pin(expected);

    REQUIRE(m.pin_count() == 1);
    CHECK(m.pin_at(0).label   == "Big Ben");
    CHECK(m.pin_at(0).address == "Westminster, London");
    CHECK(m.pin_at(0).location.latitude  == 51.5007);
    CHECK(m.pin_at(0).location.longitude == -0.1246);
}

TEST_CASE("map_view clear_pins empties collection", "[mock][map_view][pins]") {
    mpapp::internal::basic_map_view m;

    m.add_pin(mpapp::map_pin{"A", "", {1.0, 2.0}});
    m.add_pin(mpapp::map_pin{"B", "", {3.0, 4.0}});
    REQUIRE(m.pin_count() == 2);

    m.clear_pins();
    REQUIRE(m.pin_count() == 0);
}

TEST_CASE("map_view multiple pins preserve insertion order", "[mock][map_view][pins]") {
    mpapp::internal::basic_map_view m;

    m.add_pin(mpapp::map_pin{"First",  "", {1.0, 1.0}});
    m.add_pin(mpapp::map_pin{"Second", "", {2.0, 2.0}});
    m.add_pin(mpapp::map_pin{"Third",  "", {3.0, 3.0}});

    REQUIRE(m.pin_count() == 3);
    CHECK(m.pin_at(0).label == "First");
    CHECK(m.pin_at(1).label == "Second");
    CHECK(m.pin_at(2).label == "Third");
}

// ---------------------------------------------------------------------------
// pin_clicked signal
// ---------------------------------------------------------------------------

TEST_CASE("map_view pin_clicked signal fires and mock handler records it", "[mock][map_view][signals]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_pin_clicked(m);

    mpapp::map_pin p{"Test Pin", "Test Address", {10.0, 20.0}};
    m.pin_clicked.emit(p);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "pin_clicked");
}

TEST_CASE("map_view pin_clicked fires once per emit", "[mock][map_view][signals]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_pin_clicked(m);

    mpapp::map_pin p1{"Pin A", "", {1.0, 2.0}};
    mpapp::map_pin p2{"Pin B", "", {3.0, 4.0}};

    m.pin_clicked.emit(p1);
    m.pin_clicked.emit(p2);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "pin_clicked");
    CHECK(h.calls()[1].property_name == "pin_clicked");
}

TEST_CASE("map_view pin_clicked not recorded before map_pin_clicked called", "[mock][map_view][signals]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    // map_pin_clicked NOT called
    mpapp::map_pin p{"Orphan", "", {0.0, 0.0}};
    m.pin_clicked.emit(p);

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// map_clicked signal
// ---------------------------------------------------------------------------

TEST_CASE("map_view map_clicked signal fires and mock handler records it", "[mock][map_view][signals]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_map_clicked(m);

    mpapp::geo_point gp{48.8566, 2.3522};
    m.map_clicked.emit(gp);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "map_clicked");
}

TEST_CASE("map_view map_clicked fires once per emit", "[mock][map_view][signals]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_map_clicked(m);

    m.map_clicked.emit(mpapp::geo_point{0.0, 0.0});
    m.map_clicked.emit(mpapp::geo_point{1.0, 1.0});
    m.map_clicked.emit(mpapp::geo_point{2.0, 2.0});

    REQUIRE(h.calls().size() == 3);
}

TEST_CASE("map_view map_clicked not recorded before map_map_clicked called", "[mock][map_view][signals]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    m.map_clicked.emit(mpapp::geo_point{0.0, 0.0});

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// geo_point and map_pin equality
// ---------------------------------------------------------------------------

TEST_CASE("geo_point equality operator", "[mock][map_view][types]") {
    mpapp::geo_point a{1.0, 2.0};
    mpapp::geo_point b{1.0, 2.0};
    mpapp::geo_point c{3.0, 4.0};

    CHECK(a == b);
    CHECK_FALSE(a == c);
}

TEST_CASE("map_pin equality operator", "[mock][map_view][types]") {
    mpapp::map_pin a{"label", "addr", {1.0, 2.0}};
    mpapp::map_pin b{"label", "addr", {1.0, 2.0}};
    mpapp::map_pin c{"other", "addr", {1.0, 2.0}};

    CHECK(a == b);
    CHECK_FALSE(a == c);
}

// ---------------------------------------------------------------------------
// to_string for map_kind
// ---------------------------------------------------------------------------

TEST_CASE("map_kind to_string covers all enumerators", "[mock][map_view][types]") {
    CHECK(mpapp::to_string(mpapp::map_kind::street)    == "street");
    CHECK(mpapp::to_string(mpapp::map_kind::satellite) == "satellite");
    CHECK(mpapp::to_string(mpapp::map_kind::hybrid)    == "hybrid");
}

// ---------------------------------------------------------------------------
// Sequence: multiple properties + clear_calls
// ---------------------------------------------------------------------------

TEST_CASE("map_view sequence: map all properties then drive changes", "[mock][map_view][sequence]") {
    mpapp::internal::basic_map_view m;
    map_mock h;

    h.map_kind(m);
    h.map_center(m);
    h.map_zoom(m);
    h.map_is_showing_user(m);
    h.map_is_traffic_enabled(m);
    h.map_is_scroll_enabled(m);
    h.map_is_zoom_enabled(m);

    // Seven initial records.
    REQUIRE(h.calls().size() == 7);
    h.clear_calls();

    m.kind             = mpapp::map_kind::hybrid;
    m.center           = mpapp::geo_point{40.7128, -74.0060};
    m.zoom             = 12.0;
    m.is_showing_user  = true;
    m.is_traffic_enabled = true;
    m.is_scroll_enabled  = false;
    m.is_zoom_enabled    = false;

    REQUIRE(h.calls().size() == 7);
    CHECK(h.calls()[0].property_name == "kind");
    CHECK(h.calls()[1].property_name == "center");
    CHECK(h.calls()[2].property_name == "zoom");
    CHECK(h.calls()[3].property_name == "is_showing_user");
    CHECK(h.calls()[4].property_name == "is_traffic_enabled");
    CHECK(h.calls()[5].property_name == "is_scroll_enabled");
    CHECK(h.calls()[6].property_name == "is_zoom_enabled");
}
