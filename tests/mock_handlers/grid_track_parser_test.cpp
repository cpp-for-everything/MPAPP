// SPDX-License-Identifier: Apache-2.0
// Tests for track_def::parse - the ADR-0017 MAUI-string DSL parser.

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_grid_layout.hpp>

using namespace mpapp;
using kind = track_def::kind;

TEST_CASE("track_def::parse empty string -> empty vector",
          "[grid][track_def]") {
    auto v = track_def::parse("");
    CHECK(v.empty());
}

TEST_CASE("track_def::parse handles all three kinds",
          "[grid][track_def]") {
    auto v = track_def::parse("Auto, *, 200, 2*");
    REQUIRE(v.size() == 4);

    CHECK(v[0].k     == kind::auto_);

    CHECK(v[1].k     == kind::star);
    CHECK(v[1].value == 1.0);

    CHECK(v[2].k     == kind::fixed);
    CHECK(v[2].value == 200.0);

    CHECK(v[3].k     == kind::star);
    CHECK(v[3].value == 2.0);
}

TEST_CASE("track_def::parse is whitespace tolerant",
          "[grid][track_def]") {
    auto v1 = track_def::parse("Auto,*,200");
    auto v2 = track_def::parse(" Auto ,   * ,200  ");
    REQUIRE(v1.size() == 3);
    REQUIRE(v2.size() == 3);
    CHECK(v1[0] == v2[0]);
    CHECK(v1[1] == v2[1]);
    CHECK(v1[2] == v2[2]);
}

TEST_CASE("track_def::parse accepts lowercase 'auto'",
          "[grid][track_def]") {
    auto v = track_def::parse("auto, 100");
    REQUIRE(v.size() == 2);
    CHECK(v[0].k == kind::auto_);
    CHECK(v[1].k == kind::fixed);
}

TEST_CASE("track_def::parse defaults invalid star weight to 1.0",
          "[grid][track_def]") {
    auto v = track_def::parse("-1*, 0*, 1.5*");
    REQUIRE(v.size() == 3);
    CHECK(v[0].value == 1.0);   // -1 → fallback
    CHECK(v[1].value == 1.0);   // 0  → fallback
    CHECK(v[2].value == 1.5);
}

TEST_CASE("grid_layout exposes set_rows_from_spec helper",
          "[grid][grid_layout]") {
    internal::basic_grid_layout g;
    g.set_rows_from_spec("Auto, *");
    g.set_columns_from_spec("100, 100");

    REQUIRE(g.row_definitions.get().size()    == 2);
    REQUIRE(g.column_definitions.get().size() == 2);
    CHECK(g.row_definitions.get()[0].k        == kind::auto_);
    CHECK(g.row_definitions.get()[1].k        == kind::star);
    CHECK(g.column_definitions.get()[0].k     == kind::fixed);
    CHECK(g.column_definitions.get()[0].value == 100.0);
}
