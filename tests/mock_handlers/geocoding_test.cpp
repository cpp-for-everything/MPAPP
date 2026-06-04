// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0013 Essentials Geocoding API.
//
// Covers: placemark value type, mock_geocoding forward/reverse geocoding,
// default/not-found paths, query-recording accessors, and multiple results.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/geocoding.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// placemark - value type
// ---------------------------------------------------------------------------

TEST_CASE("placemark: default construction has empty strings and zero coords",
          "[mock][geocoding][placemark]") {
    // Arrange / Act
    placemark p;

    // Assert
    CHECK(p.country_name.empty());
    CHECK(p.country_code.empty());
    CHECK(p.admin_area.empty());
    CHECK(p.sub_admin_area.empty());
    CHECK(p.locality.empty());
    CHECK(p.sub_locality.empty());
    CHECK(p.thoroughfare.empty());
    CHECK(p.postal_code.empty());
    CHECK(p.feature_name.empty());
    CHECK(p.latitude  == 0.0);
    CHECK(p.longitude == 0.0);
}

TEST_CASE("placemark: equality when all fields match", "[mock][geocoding][placemark]") {
    // Arrange
    placemark a;
    a.country_name  = "United Kingdom";
    a.country_code  = "GB";
    a.admin_area    = "England";
    a.sub_admin_area = "Greater London";
    a.locality      = "London";
    a.sub_locality  = "City of London";
    a.thoroughfare  = "Downing Street";
    a.postal_code   = "SW1A 2AA";
    a.feature_name  = "10 Downing Street";
    a.latitude      = 51.503399;
    a.longitude     = -0.127616;

    placemark b = a;  // copy

    // Act / Assert
    CHECK(a == b);
}

TEST_CASE("placemark: inequality when any field differs",
          "[mock][geocoding][placemark]") {
    // Arrange
    placemark a;
    a.country_name = "Germany";
    a.latitude     = 52.52;
    a.longitude    = 13.405;

    placemark b = a;
    b.country_name = "France";

    // Act / Assert
    CHECK_FALSE(a == b);
}

TEST_CASE("placemark: inequality when latitude differs",
          "[mock][geocoding][placemark]") {
    // Arrange
    placemark a;
    a.latitude  = 48.8566;
    a.longitude = 2.3522;

    placemark b = a;
    b.latitude  = 51.5074;

    // Act / Assert
    CHECK_FALSE(a == b);
}

// ---------------------------------------------------------------------------
// mock_geocoding - forward geocoding (get_locations_for_address)
// ---------------------------------------------------------------------------

TEST_CASE("mock_geocoding: get_locations_for_address returns empty for unknown address",
          "[mock][geocoding][forward]") {
    // Arrange
    mock_geocoding geo;

    // Act
    auto results = geo.get_locations_for_address("nowhere");

    // Assert
    CHECK(results.empty());
}

TEST_CASE("mock_geocoding: get_locations_for_address returns registered results",
          "[mock][geocoding][forward]") {
    // Arrange
    mock_geocoding geo;

    placemark p;
    p.country_name = "United States";
    p.country_code = "US";
    p.locality     = "New York";
    p.latitude     = 40.7128;
    p.longitude    = -74.0060;

    geo.register_address("New York, NY", { p });

    // Act
    auto results = geo.get_locations_for_address("New York, NY");

    // Assert
    REQUIRE(results.size() == 1);
    CHECK(results[0] == p);
}

TEST_CASE("mock_geocoding: get_locations_for_address returns multiple results",
          "[mock][geocoding][forward]") {
    // Arrange
    mock_geocoding geo;

    placemark p1;
    p1.locality  = "Springfield";
    p1.admin_area = "Illinois";
    p1.latitude   = 39.7817;
    p1.longitude  = -89.6501;

    placemark p2;
    p2.locality  = "Springfield";
    p2.admin_area = "Missouri";
    p2.latitude   = 37.2153;
    p2.longitude  = -93.2982;

    geo.register_address("Springfield", { p1, p2 });

    // Act
    auto results = geo.get_locations_for_address("Springfield");

    // Assert
    REQUIRE(results.size() == 2);
    CHECK(results[0] == p1);
    CHECK(results[1] == p2);
}

TEST_CASE("mock_geocoding: get_locations_for_address records last query",
          "[mock][geocoding][forward]") {
    // Arrange
    mock_geocoding geo;

    // Act
    (void)geo.get_locations_for_address("10 Downing Street, London");

    // Assert
    CHECK(geo.last_address_query() == "10 Downing Street, London");
}

TEST_CASE("mock_geocoding: last_address_query updates on each call",
          "[mock][geocoding][forward]") {
    // Arrange
    mock_geocoding geo;

    // Act
    (void)geo.get_locations_for_address("First Street");
    (void)geo.get_locations_for_address("Second Avenue");

    // Assert - only the latest query is retained
    CHECK(geo.last_address_query() == "Second Avenue");
}

TEST_CASE("mock_geocoding: last_address_query is empty before any call",
          "[mock][geocoding][forward]") {
    // Arrange
    mock_geocoding geo;

    // Act / Assert
    CHECK(geo.last_address_query().empty());
}

TEST_CASE("mock_geocoding: re-registering an address replaces previous results",
          "[mock][geocoding][forward]") {
    // Arrange
    mock_geocoding geo;

    placemark old_p;
    old_p.locality = "OldTown";
    geo.register_address("Main St", { old_p });

    placemark new_p;
    new_p.locality = "NewTown";
    geo.register_address("Main St", { new_p });

    // Act
    auto results = geo.get_locations_for_address("Main St");

    // Assert
    REQUIRE(results.size() == 1);
    CHECK(results[0].locality == "NewTown");
}

// ---------------------------------------------------------------------------
// mock_geocoding - reverse geocoding (get_placemarks)
// ---------------------------------------------------------------------------

TEST_CASE("mock_geocoding: get_placemarks returns empty for unknown coordinates",
          "[mock][geocoding][reverse]") {
    // Arrange
    mock_geocoding geo;

    // Act
    auto results = geo.get_placemarks(0.0, 0.0);

    // Assert
    CHECK(results.empty());
}

TEST_CASE("mock_geocoding: get_placemarks returns registered results",
          "[mock][geocoding][reverse]") {
    // Arrange
    mock_geocoding geo;

    placemark p;
    p.country_name = "France";
    p.country_code = "FR";
    p.locality     = "Paris";
    p.latitude     = 48.8566;
    p.longitude    = 2.3522;

    geo.register_coordinates(48.8566, 2.3522, { p });

    // Act
    auto results = geo.get_placemarks(48.8566, 2.3522);

    // Assert
    REQUIRE(results.size() == 1);
    CHECK(results[0] == p);
}

TEST_CASE("mock_geocoding: get_placemarks returns multiple results",
          "[mock][geocoding][reverse]") {
    // Arrange
    mock_geocoding geo;

    placemark p1;
    p1.feature_name = "Eiffel Tower";
    p1.locality     = "Paris";

    placemark p2;
    p2.feature_name = "7th Arrondissement";
    p2.locality     = "Paris";

    geo.register_coordinates(48.8584, 2.2945, { p1, p2 });

    // Act
    auto results = geo.get_placemarks(48.8584, 2.2945);

    // Assert
    REQUIRE(results.size() == 2);
    CHECK(results[0].feature_name == "Eiffel Tower");
    CHECK(results[1].feature_name == "7th Arrondissement");
}

TEST_CASE("mock_geocoding: get_placemarks records last query coordinates",
          "[mock][geocoding][reverse]") {
    // Arrange
    mock_geocoding geo;

    // Act
    (void)geo.get_placemarks(51.5074, -0.1278);

    // Assert
    CHECK(geo.last_latitude_query()  == 51.5074);
    CHECK(geo.last_longitude_query() == -0.1278);
}

TEST_CASE("mock_geocoding: last coordinate query updates on each call",
          "[mock][geocoding][reverse]") {
    // Arrange
    mock_geocoding geo;

    // Act
    (void)geo.get_placemarks(10.0, 20.0);
    (void)geo.get_placemarks(30.0, 40.0);

    // Assert - only the latest coordinates are retained
    CHECK(geo.last_latitude_query()  == 30.0);
    CHECK(geo.last_longitude_query() == 40.0);
}

TEST_CASE("mock_geocoding: last coordinate queries are 0.0 before any call",
          "[mock][geocoding][reverse]") {
    // Arrange
    mock_geocoding geo;

    // Act / Assert
    CHECK(geo.last_latitude_query()  == 0.0);
    CHECK(geo.last_longitude_query() == 0.0);
}

TEST_CASE("mock_geocoding: re-registering coordinates replaces previous results",
          "[mock][geocoding][reverse]") {
    // Arrange
    mock_geocoding geo;

    placemark old_p;
    old_p.locality = "OldPlace";
    geo.register_coordinates(1.0, 2.0, { old_p });

    placemark new_p;
    new_p.locality = "NewPlace";
    geo.register_coordinates(1.0, 2.0, { new_p });

    // Act
    auto results = geo.get_placemarks(1.0, 2.0);

    // Assert
    REQUIRE(results.size() == 1);
    CHECK(results[0].locality == "NewPlace");
}

// ---------------------------------------------------------------------------
// mock_geocoding - cross-method: independent address and coordinate tables
// ---------------------------------------------------------------------------

TEST_CASE("mock_geocoding: address and coordinate tables are independent",
          "[mock][geocoding][cross]") {
    // Arrange
    mock_geocoding geo;

    placemark addr_p;
    addr_p.locality = "FromAddress";

    placemark coord_p;
    coord_p.locality = "FromCoords";

    geo.register_address("test address", { addr_p });
    geo.register_coordinates(10.0, 20.0, { coord_p });

    // Act
    auto addr_results  = geo.get_locations_for_address("test address");
    auto coord_results = geo.get_placemarks(10.0, 20.0);

    // Assert
    REQUIRE(addr_results.size()  == 1);
    REQUIRE(coord_results.size() == 1);
    CHECK(addr_results[0].locality  == "FromAddress");
    CHECK(coord_results[0].locality == "FromCoords");
}

TEST_CASE("mock_geocoding: forward query does not affect coordinate query",
          "[mock][geocoding][cross]") {
    // Arrange
    mock_geocoding geo;

    // Act
    (void)geo.get_locations_for_address("somewhere");

    // Assert - coordinate query state untouched
    CHECK(geo.last_latitude_query()  == 0.0);
    CHECK(geo.last_longitude_query() == 0.0);
}

TEST_CASE("mock_geocoding: reverse query does not affect address query",
          "[mock][geocoding][cross]") {
    // Arrange
    mock_geocoding geo;

    // Act
    (void)geo.get_placemarks(5.0, 6.0);

    // Assert - address query state untouched
    CHECK(geo.last_address_query().empty());
}

// ---------------------------------------------------------------------------
// mock_geocoding - interface pointer usage (polymorphic dispatch)
// ---------------------------------------------------------------------------

TEST_CASE("mock_geocoding: usable through base geocoding pointer",
          "[mock][geocoding][poly]") {
    // Arrange
    mock_geocoding mock;

    placemark p;
    p.country_name = "Japan";
    p.locality     = "Tokyo";
    p.latitude     = 35.6895;
    p.longitude    = 139.6917;

    mock.register_address("Tokyo, Japan", { p });
    mock.register_coordinates(35.6895, 139.6917, { p });

    geocoding* geo = &mock;

    // Act
    auto fwd = geo->get_locations_for_address("Tokyo, Japan");
    auto rev = geo->get_placemarks(35.6895, 139.6917);

    // Assert
    REQUIRE(fwd.size() == 1);
    REQUIRE(rev.size() == 1);
    CHECK(fwd[0].country_name == "Japan");
    CHECK(rev[0].locality     == "Tokyo");
}

// ---------------------------------------------------------------------------
// mock_geocoding - empty registration (register empty vector)
// ---------------------------------------------------------------------------

TEST_CASE("mock_geocoding: registering empty vector returns empty results",
          "[mock][geocoding][empty]") {
    // Arrange
    mock_geocoding geo;
    geo.register_address("ghost town", {});
    geo.register_coordinates(99.0, 99.0, {});

    // Act
    auto fwd = geo.get_locations_for_address("ghost town");
    auto rev = geo.get_placemarks(99.0, 99.0);

    // Assert - registered but empty; query was still recorded
    CHECK(fwd.empty());
    CHECK(rev.empty());
    CHECK(geo.last_address_query() == "ghost town");
    CHECK(geo.last_latitude_query()  == 99.0);
    CHECK(geo.last_longitude_query() == 99.0);
}
