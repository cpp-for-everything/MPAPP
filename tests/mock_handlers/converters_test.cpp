// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0007 value-converters library.

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/binding.hpp>
#include <mpapp/binding/converters.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

TEST_CASE("invert_bool_converter + helper negate", "[mock][converters]") {
    invert_bool_converter c;
    CHECK(c.convert(true) == false);
    CHECK(c.convert(false) == true);
    CHECK(c.convert_back(true) == false);

    auto fn = invert_bool();
    CHECK(fn(true) == false);
    CHECK(fn(false) == true);
}

TEST_CASE("bool_to_visibility_converter maps both ways", "[mock][converters]") {
    bool_to_visibility_converter c;             // collapse-when-false
    CHECK(c.convert(true)  == visibility::visible);
    CHECK(c.convert(false) == visibility::collapsed);
    CHECK(c.convert_back(visibility::visible) == true);
    CHECK(c.convert_back(visibility::hidden)  == false);

    bool_to_visibility_converter h{ false };    // hide-when-false
    CHECK(h.convert(false) == visibility::hidden);

    auto fn = bool_to_visibility();
    CHECK(fn(true)  == visibility::visible);
    CHECK(fn(false) == visibility::collapsed);
}

TEST_CASE("format_with applies a runtime std::format pattern",
          "[mock][converters]") {
    auto money = format_with<double>("${:.2f}");
    CHECK(money(3.5)   == "$3.50");
    CHECK(money(10.0)  == "$10.00");

    auto padded = format_with<int>("[{:03}]");
    CHECK(padded(7) == "[007]");

    CHECK(to_string_converter<int>()(42) == "42");
}

TEST_CASE("converters plug into a binding's converter slot",
          "[mock][converters][binding]") {
    // Source: a bool "is busy"; target: a string caption, via format on
    // the inverted value (idle when not busy).
    Observable<bool>        busy{ false };
    Observable<std::string> caption{ "" };

    binding<bool, std::string> b{
        busy, caption, binding_mode::one_way,
        [](const bool& is_busy) { return is_busy ? "Working…" : "Idle"; }
    };

    CHECK(caption.get() == "Idle");
    busy = true;
    CHECK(caption.get() == "Working…");

    // A numeric source formatted into the target via format_with.
    Observable<double>      ratio{ 0.25 };
    Observable<std::string> pct{ "" };
    binding<double, std::string> pb{
        ratio, pct, binding_mode::one_way, format_with<double>("{:.0f}%")
    };
    CHECK(pct.get() == "0%");                    // 0.25 -> "0%" (no *100; raw value)
    ratio = 90.0;
    CHECK(pct.get() == "90%");
}
