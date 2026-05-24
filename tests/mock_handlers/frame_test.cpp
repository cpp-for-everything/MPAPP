// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Frame.md
//
// Mock-handler tests for the **deprecated** `mpapp::internal::basic_frame` control.
// (CLAUDE Rule 6 / ADR-0008.) The deprecation diagnostic is suppressed
// locally so the test target compiles with `-Werror`. New code should
// use `mpapp::internal::basic_border` — Frame is retained for one-to-one XAML
// compatibility with Forms / early MAUI codebases.

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#endif

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include <mpapp/frame.hpp>
#include <mpapp/handlers/mock/frame_handler.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class plain_view : public view {};

} // namespace

TEST_CASE("frame mock handler records initial values on bind",
          "[mock][frame][deprecated]") {
    internal::basic_frame f;
    frame_handler<platform::mock> h;

    h.map_has_shadow(f);
    h.map_corner_radius(f);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "has_shadow");
    CHECK(h.calls()[0].value_repr    == "true");      // MAUI default
    CHECK(h.calls()[1].property_name == "corner_radius");
    CHECK(h.calls()[1].value_repr    == "-1");        // sentinel: "platform default"
}

TEST_CASE("frame mock handler records single call per property change",
          "[mock][frame]") {
    internal::basic_frame f;
    frame_handler<platform::mock> h;

    h.map_has_shadow(f);
    h.clear_calls();

    f.has_shadow = false;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "false");

    f.has_shadow = false;                 // idempotent
    REQUIRE(h.calls().size() == 1);

    f.has_shadow = true;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "true");
}

TEST_CASE("frame mock handler tracks content presence",
          "[mock][frame]") {
    internal::basic_frame f;
    frame_handler<platform::mock> h;

    h.map_content(f);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "false");

    f.content = std::make_shared<plain_view>();
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "true");
}

TEST_CASE("frame default padding mirrors MAUI's 20-dip default",
          "[mock][frame][padding]") {
    internal::basic_frame f;
    frame_handler<platform::mock> h;

    h.map_padding(f);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "padding");
    CHECK(h.calls()[0].value_repr    == "thickness(20,20,20,20)");
}

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif
