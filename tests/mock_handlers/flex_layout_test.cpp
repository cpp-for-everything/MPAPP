// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_flex_layout`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_flex_layout.hpp>
#include <mpapp/handlers/mock/flex_layout_handler.hpp>

using namespace mpapp;

namespace {
class plain_view : public view {};
} // namespace

TEST_CASE("flex_layout enum to_string round-trips every value",
          "[mock][flex_layout]") {
    // Arrange / Act / Assert - exhaustive coverage of every to_string arm.
    CHECK(to_string(flex_direction::row)            == "row");
    CHECK(to_string(flex_direction::row_reverse)    == "row_reverse");
    CHECK(to_string(flex_direction::column)         == "column");
    CHECK(to_string(flex_direction::column_reverse) == "column_reverse");

    CHECK(to_string(flex_wrap::no_wrap)      == "no_wrap");
    CHECK(to_string(flex_wrap::wrap)         == "wrap");
    CHECK(to_string(flex_wrap::wrap_reverse) == "wrap_reverse");

    CHECK(to_string(flex_justify::start)         == "start");
    CHECK(to_string(flex_justify::center)        == "center");
    CHECK(to_string(flex_justify::end)           == "end");
    CHECK(to_string(flex_justify::space_between) == "space_between");
    CHECK(to_string(flex_justify::space_around)  == "space_around");
    CHECK(to_string(flex_justify::space_evenly)  == "space_evenly");

    CHECK(to_string(flex_align_items::stretch) == "stretch");
    CHECK(to_string(flex_align_items::center)  == "center");
    CHECK(to_string(flex_align_items::start)   == "start");
    CHECK(to_string(flex_align_items::end)     == "end");

    CHECK(to_string(flex_align_content::stretch)       == "stretch");
    CHECK(to_string(flex_align_content::center)        == "center");
    CHECK(to_string(flex_align_content::start)         == "start");
    CHECK(to_string(flex_align_content::end)           == "end");
    CHECK(to_string(flex_align_content::space_between) == "space_between");
    CHECK(to_string(flex_align_content::space_around)  == "space_around");

    CHECK(to_string(flex_align_self::auto_)   == "auto");
    CHECK(to_string(flex_align_self::stretch) == "stretch");
    CHECK(to_string(flex_align_self::center)  == "center");
    CHECK(to_string(flex_align_self::start)   == "start");
    CHECK(to_string(flex_align_self::end)     == "end");

    CHECK(to_string(flex_position::relative) == "relative");
    CHECK(to_string(flex_position::absolute) == "absolute");
}

TEST_CASE("flex_layout to_string returns '?' for out-of-range values",
          "[mock][flex_layout]") {
    // Arrange - values outside every defined enumerator hit the fallthrough.
    CHECK(to_string(static_cast<flex_direction>(99))     == "?");
    CHECK(to_string(static_cast<flex_wrap>(99))          == "?");
    CHECK(to_string(static_cast<flex_justify>(99))       == "?");
    CHECK(to_string(static_cast<flex_align_items>(99))   == "?");
    CHECK(to_string(static_cast<flex_align_content>(99)) == "?");
    CHECK(to_string(static_cast<flex_align_self>(99))    == "?");
    CHECK(to_string(static_cast<flex_position>(99))      == "?");
}

TEST_CASE("flex_layout exposes the expected container defaults",
          "[mock][flex_layout]") {
    // Arrange / Act
    internal::basic_flex_layout f;

    // Assert - MAUI FlexLayout defaults.
    CHECK(f.direction.get()       == flex_direction::row);
    CHECK(f.wrap.get()            == flex_wrap::no_wrap);
    CHECK(f.justify_content.get() == flex_justify::start);
    CHECK(f.align_items.get()     == flex_align_items::stretch);
    CHECK(f.align_content.get()   == flex_align_content::stretch);
    CHECK(f.position.get()        == flex_position::relative);
}

TEST_CASE("flex_layout mock handler records initial property values on bind",
          "[mock][flex_layout]") {
    // Arrange
    internal::basic_flex_layout f;
    internal::flex_layout_handler<platform::mock> h;

    // Act
    h.map_direction(f);
    h.map_wrap(f);
    h.map_justify_content(f);
    h.map_align_items(f);
    h.map_align_content(f);
    h.map_position(f);

    // Assert
    REQUIRE(h.calls().size() == 6);
    CHECK(h.calls()[0].property_name == "direction");
    CHECK(h.calls()[0].value_repr    == "row");
    CHECK(h.calls()[1].property_name == "wrap");
    CHECK(h.calls()[1].value_repr    == "no_wrap");
    CHECK(h.calls()[2].property_name == "justify_content");
    CHECK(h.calls()[2].value_repr    == "start");
    CHECK(h.calls()[3].property_name == "align_items");
    CHECK(h.calls()[3].value_repr    == "stretch");
    CHECK(h.calls()[4].property_name == "align_content");
    CHECK(h.calls()[4].value_repr    == "stretch");
    CHECK(h.calls()[5].property_name == "position");
    CHECK(h.calls()[5].value_repr    == "relative");
}

TEST_CASE("flex_layout mock handler fires once per real property change",
          "[mock][flex_layout]") {
    // Arrange
    internal::basic_flex_layout f;
    internal::flex_layout_handler<platform::mock> h;

    h.map_direction(f);
    h.map_wrap(f);
    h.map_justify_content(f);
    h.map_align_items(f);
    h.map_align_content(f);
    h.map_position(f);
    h.clear_calls();

    // Act
    f.direction       = flex_direction::column;
    f.wrap            = flex_wrap::wrap;
    f.wrap            = flex_wrap::wrap;       // suppressed - no real change
    f.justify_content = flex_justify::space_between;
    f.align_items     = flex_align_items::center;
    f.align_content   = flex_align_content::space_around;
    f.position        = flex_position::absolute;

    // Assert
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "direction=column",
        "wrap=wrap",
        "justify_content=space_between",
        "align_items=center",
        "align_content=space_around",
        "position=absolute",
    });
}

TEST_CASE("flex_layout child attached props default correctly",
          "[mock][flex_layout]") {
    // Arrange
    internal::basic_flex_layout f;
    plain_view child;

    // Act - never set anything for this child.
    auto props = f.get_child_props(child);

    // Assert - MAUI defaults: order 0, grow 0, shrink 1, align_self auto,
    // basis -1 (auto).
    CHECK(props.order      == 0);
    CHECK(props.grow       == 0.0);
    CHECK(props.shrink     == 1.0);
    CHECK(props.align_self == flex_align_self::auto_);
    CHECK(props.basis      == -1.0);
}

TEST_CASE("flex_layout stores and retrieves per-child attached props",
          "[mock][flex_layout]") {
    // Arrange
    internal::basic_flex_layout f;
    plain_view a;
    plain_view b;

    // Act
    f.set_order(a, 3);
    f.set_grow(a, 2.0);
    f.set_shrink(a, 0.5);
    f.set_align_self(a, flex_align_self::center);
    f.set_basis(a, 120.0);

    // Assert - a holds its values; b is untouched (defaults).
    auto pa = f.get_child_props(a);
    CHECK(pa.order      == 3);
    CHECK(pa.grow       == 2.0);
    CHECK(pa.shrink     == 0.5);
    CHECK(pa.align_self == flex_align_self::center);
    CHECK(pa.basis      == 120.0);

    auto pb = f.get_child_props(b);
    CHECK(pb.order      == 0);
    CHECK(pb.grow       == 0.0);
    CHECK(pb.shrink     == 1.0);
    CHECK(pb.align_self == flex_align_self::auto_);
    CHECK(pb.basis      == -1.0);
}

TEST_CASE("flex_layout mock handler records resolved child attached props",
          "[mock][flex_layout]") {
    // Arrange
    internal::basic_flex_layout f;
    internal::flex_layout_handler<platform::mock> h;
    plain_view child;

    f.set_order(child, 5);
    f.set_grow(child, 1.5);
    f.set_shrink(child, 0.0);
    f.set_align_self(child, flex_align_self::end);
    f.set_basis(child, 64.0);

    // Act
    h.set_order(f, child);
    h.set_grow(f, child);
    h.set_shrink(f, child);
    h.set_align_self(f, child);
    h.set_basis(f, child);

    // Assert
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "order=5",
        "grow=1.5",
        "shrink=0",
        "align_self=end",
        "basis=64",
    });
}

TEST_CASE("flex_layout mock handler records child-list commands",
          "[mock][flex_layout]") {
    // Arrange
    internal::basic_flex_layout f;
    internal::flex_layout_handler<platform::mock> h;
    plain_view child;

    // Act
    h.map_add(f, child);
    h.map_remove(f, child);
    h.map_clear(f);

    // Assert - bare events, no value payload.
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "add",
        "remove",
        "clear",
    });
}

TEST_CASE("flex_layout mock handler map_gestures is a no-op",
          "[mock][flex_layout]") {
    // Arrange
    internal::basic_flex_layout f;
    internal::flex_layout_handler<platform::mock> h;

    // Act
    h.map_gestures(f);

    // Assert - stub records nothing.
    CHECK(h.calls().empty());
}

TEST_CASE("flex_layout is unbound from any handler by default",
          "[mock][flex_layout]") {
    // Arrange / Act - a bare surface has no platform handler attached.
    // (set_handler / handler() take a platform::current handler and are
    // exercised through the wrapper on a real platform, not the mock
    // surface - mirroring grid_layout_test staying on the surface.)
    internal::basic_flex_layout f;

    // Assert
    CHECK_FALSE(f.has_handler());
}
