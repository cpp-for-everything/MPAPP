// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_toast`.
//
// Drives `basic_toast` via `toast_handler<platform::mock>` and asserts
// that every property change and signal emission is recorded exactly
// once. Tests follow the AAA pattern and use the Catch2 macro API.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_toast.hpp>
#include <mpapp/handlers/mock/toast_handler.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// text property
// ---------------------------------------------------------------------------

TEST_CASE("toast map_text records initial value on attach", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_text(t);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text="});
}

TEST_CASE("toast map_text fires once per real text change", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_text(t);
    h.clear_calls();

    t.text = "Hello toast";

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text=Hello toast"});
}

TEST_CASE("toast map_text ignores same-value text writes", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    t.text = "stable";
    h.map_text(t);
    h.clear_calls();

    t.text = "stable";

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// duration property
// ---------------------------------------------------------------------------

TEST_CASE("toast map_duration records initial value short", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_duration(t);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"duration=short"});
}

TEST_CASE("toast map_duration records change to long", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_duration(t);
    h.clear_calls();

    t.duration = toast_duration::long_;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"duration=long"});
}

TEST_CASE("toast map_duration ignores same-value duration writes", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_duration(t);
    h.clear_calls();

    t.duration = toast_duration::short_; // already short_

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// is_shown property
// ---------------------------------------------------------------------------

TEST_CASE("toast map_is_shown records initial false", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_is_shown(t);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_shown=false"});
}

TEST_CASE("toast map_is_shown records change when show() is called", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_is_shown(t);
    h.clear_calls();

    t.show();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_shown=true"});
}

TEST_CASE("toast map_is_shown records change when dismiss() is called", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_is_shown(t);
    t.show();
    h.clear_calls();

    t.dismiss();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_shown=false"});
}

// ---------------------------------------------------------------------------
// show() / dismiss() idempotency
// ---------------------------------------------------------------------------

TEST_CASE("toast show() is idempotent", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_is_shown(t);
    t.show();
    h.clear_calls();

    t.show(); // already shown -- no-op

    REQUIRE(h.calls().empty());
}

TEST_CASE("toast dismiss() is idempotent", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_is_shown(t);
    h.clear_calls();

    t.dismiss(); // already dismissed (is_shown=false) -- no-op

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// shown signal
// ---------------------------------------------------------------------------

TEST_CASE("toast map_shown records shown event on show()", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_shown(t);
    t.show();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"shown"});
}

TEST_CASE("toast shown signal fires once per show call", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_shown(t);
    t.show();
    t.dismiss();
    t.show();

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"shown", "shown"});
}

TEST_CASE("toast shown signal does not fire on idempotent show()", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_shown(t);
    t.show();
    h.clear_calls();

    t.show(); // already shown -- no-op, no signal

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// dismissed signal
// ---------------------------------------------------------------------------

TEST_CASE("toast map_dismissed records dismissed event on dismiss()", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_dismissed(t);
    t.show();
    t.dismiss();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"dismissed"});
}

TEST_CASE("toast dismissed signal fires once per dismiss call", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_dismissed(t);
    t.show();
    t.dismiss();
    t.show();
    t.dismiss();

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"dismissed", "dismissed"});
}

TEST_CASE("toast dismissed signal does not fire on idempotent dismiss()", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_dismissed(t);
    // t is not shown -- dismiss() is a no-op

    t.dismiss();

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// combined: all mappers together
// ---------------------------------------------------------------------------

TEST_CASE("toast all mappers record initial state together", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_text(t);
    h.map_duration(t);
    h.map_is_shown(t);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "text");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "duration");
    CHECK(h.calls()[1].value_repr    == "short");
    CHECK(h.calls()[2].property_name == "is_shown");
    CHECK(h.calls()[2].value_repr    == "false");
}

TEST_CASE("toast full lifecycle: map all, show, dismiss", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    t.text     = "Operation complete";
    t.duration = toast_duration::long_;

    h.map_text(t);
    h.map_duration(t);
    h.map_is_shown(t);
    h.map_shown(t);
    h.map_dismissed(t);
    h.clear_calls();

    t.show();
    t.dismiss();

    // is_shown=true, shown, is_shown=false, dismissed
    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[0].property_name == "is_shown");
    CHECK(h.calls()[0].value_repr    == "true");
    CHECK(h.calls()[1].property_name == "shown");
    CHECK(!h.calls()[1].has_value);
    CHECK(h.calls()[2].property_name == "is_shown");
    CHECK(h.calls()[2].value_repr    == "false");
    CHECK(h.calls()[3].property_name == "dismissed");
    CHECK(!h.calls()[3].has_value);
}

TEST_CASE("toast text change after show is recorded", "[mock][toast]") {
    internal::basic_toast t;
    internal::toast_handler<platform::mock> h;

    h.map_text(t);
    h.map_is_shown(t);
    h.clear_calls();

    t.show();
    t.text = "Updated message";

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "is_shown");
    CHECK(h.calls()[1].property_name == "text");
    CHECK(h.calls()[1].value_repr    == "Updated message");
}
