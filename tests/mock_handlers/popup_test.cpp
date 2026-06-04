// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for the `basic_popup` surface and
// `popup_handler<platform::mock>`. Covers every property, the close()
// method, and both signals (opened / closed).

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_popup.hpp>
#include <mpapp/handlers/mock/popup_handler.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

// Minimal concrete view child for content-slot tests.
struct popup_child : view {
    popup_child() = default;
};

using popup_mock = internal::popup_handler<platform::mock>;

} // anonymous namespace

// ---------------------------------------------------------------------------
// content property
// ---------------------------------------------------------------------------

TEST_CASE("popup mock records initial content as null", "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_content(p);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "content");
    CHECK(h.calls()[0].value_repr    == "null");
}

TEST_CASE("popup mock records content set to a view", "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_content(p);
    h.clear_calls();

    p.content = std::make_shared<popup_child>();

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "content");
    CHECK(h.calls()[0].value_repr    == "set");
}

TEST_CASE("popup mock records content cleared to null", "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    p.content = std::make_shared<popup_child>();
    h.map_content(p);
    h.clear_calls();

    p.content = std::shared_ptr<view>{};

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "null");
}

TEST_CASE("popup mock records initial content as set when child present",
          "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    p.content = std::make_shared<popup_child>();
    h.map_content(p);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "set");
}

// ---------------------------------------------------------------------------
// is_open property
// ---------------------------------------------------------------------------

TEST_CASE("popup mock records initial is_open as false", "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_is_open(p);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_open=false"});
}

TEST_CASE("popup mock records is_open change to true", "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_is_open(p);
    h.clear_calls();

    p.is_open = true;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_open=true"});
}

TEST_CASE("popup mock ignores same-value is_open write", "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_is_open(p);
    h.clear_calls();

    p.is_open = false; // same as default

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// can_be_dismissed_by_tapping_outside property
// ---------------------------------------------------------------------------

TEST_CASE("popup mock records initial can_be_dismissed as true",
          "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_can_be_dismissed_by_tapping_outside(p);

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"can_be_dismissed_by_tapping_outside=true"});
}

TEST_CASE("popup mock records can_be_dismissed change to false",
          "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_can_be_dismissed_by_tapping_outside(p);
    h.clear_calls();

    p.can_be_dismissed_by_tapping_outside = false;

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"can_be_dismissed_by_tapping_outside=false"});
}

TEST_CASE("popup mock ignores same-value can_be_dismissed write",
          "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_can_be_dismissed_by_tapping_outside(p);
    h.clear_calls();

    p.can_be_dismissed_by_tapping_outside = true; // same as default

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// opened signal
// ---------------------------------------------------------------------------

TEST_CASE("popup mock records opened signal emission", "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_opened(p);
    p.opened.emit();
    p.opened.emit();

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"opened", "opened"});
}

TEST_CASE("popup mock does not record opened before subscription",
          "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    p.opened.emit(); // no subscription yet

    REQUIRE(h.calls().empty());
}

// ---------------------------------------------------------------------------
// closed signal and close() method
// ---------------------------------------------------------------------------

TEST_CASE("popup mock records closed signal with result string",
          "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_closed(p);
    p.close(std::string("ok"));

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "closed");
    CHECK(h.calls()[0].value_repr    == "ok");
}

TEST_CASE("popup mock records closed signal with nullopt", "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    h.map_closed(p);
    p.close(); // no result

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "closed");
    CHECK(h.calls()[0].value_repr    == "<nullopt>");
}

TEST_CASE("popup close sets is_open to false", "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    p.is_open = true;
    h.map_is_open(p);
    h.clear_calls();

    p.close(std::string("done"));

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_open=false"});
}

TEST_CASE("popup result getter returns stored result after close",
          "[mock][popup]") {
    internal::basic_popup p;

    p.close(std::string("my-result"));

    REQUIRE(p.result().has_value());
    CHECK(p.result().value() == "my-result");
}

TEST_CASE("popup result getter returns nullopt before close", "[mock][popup]") {
    internal::basic_popup p;

    CHECK(!p.result().has_value());
}

TEST_CASE("popup result getter returns nullopt after close with no result",
          "[mock][popup]") {
    internal::basic_popup p;

    p.close(); // no argument

    CHECK(!p.result().has_value());
}

// ---------------------------------------------------------------------------
// Combined: is_open + closed together
// ---------------------------------------------------------------------------

TEST_CASE("popup close emits closed and sets is_open together",
          "[mock][popup]") {
    internal::basic_popup p;
    popup_mock            h;

    p.is_open = true;
    h.map_is_open(p);
    h.map_closed(p);
    h.clear_calls();

    p.close(std::string("combined"));

    // is_open=false must appear (Observable fires) and closed=combined too.
    const auto strs = h.calls_as_strings();
    REQUIRE(strs.size() == 2);
    // Order: is_open fires when Observable is set, then closed is emitted.
    CHECK(strs[0] == "is_open=false");
    CHECK(strs[1] == "closed=combined");
}

// ---------------------------------------------------------------------------
// Handler attachment (has_handler only -- set_handler requires platform::current)
// ---------------------------------------------------------------------------

TEST_CASE("popup has_handler returns false without set_handler",
          "[mock][popup]") {
    internal::basic_popup p;
    CHECK(!p.has_handler());
}
