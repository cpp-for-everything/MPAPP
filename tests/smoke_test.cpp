#include <catch2/catch_test_macros.hpp>

#include <mpapp/mpapp.hpp>

TEST_CASE("mpapp umbrella header is reachable", "[smoke]") {
    using namespace mpapp;
    REQUIRE(true);
}
