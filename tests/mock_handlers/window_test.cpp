// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_window` (T-0011).
//
// Validates the window property-mapper contract and the imperative
// commands (`show()` / `close()`) plus the activated/closed signal
// propagation.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_button.hpp>
#include <mpapp/handlers/mock/window_handler.hpp>
#include <mpapp/internal/basic_window.hpp>

using namespace mpapp;

TEST_CASE("window mock handler records initial property values on bind",
          "[mock][window]") {
    internal::basic_window w;
    internal::window_handler<platform::mock> h;

    h.map_title(w);
    h.map_content(w);
    h.map_width(w);
    h.map_height(w);
    h.map_is_visible(w);

    REQUIRE(h.calls().size() == 5);
    CHECK(h.calls()[0].property_name == "title");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "content.present");
    CHECK(h.calls()[1].value_repr    == "false");
    CHECK(h.calls()[2].property_name == "width");
    CHECK(h.calls()[2].value_repr    == "0");
    CHECK(h.calls()[3].property_name == "height");
    CHECK(h.calls()[3].value_repr    == "0");
    CHECK(h.calls()[4].property_name == "is_visible");
    CHECK(h.calls()[4].value_repr    == "false");
}

TEST_CASE("window mock handler fires once per real title change",
          "[mock][window]") {
    internal::basic_window w;
    internal::window_handler<platform::mock> h;

    h.map_title(w);
    h.clear_calls();

    w.title = "Hello";
    w.title = "Hello";    // same value - Observable suppresses
    w.title = "World";

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"title=Hello", "title=World"});
}

TEST_CASE("window mock handler records content.present transition when assigned a view",
          "[mock][window]") {
    internal::basic_button some_button{};
    internal::basic_window w;
    internal::window_handler<platform::mock> h;

    h.map_content(w);
    h.clear_calls();

    w.content = &some_button;
    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"content.present=true"});

    w.content = nullptr;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "content.present=true", "content.present=false"});
}

TEST_CASE("window::show toggles is_visible and the handler records it",
          "[mock][window]") {
    internal::basic_window w;
    internal::window_handler<platform::mock> h;

    h.map_is_visible(w);
    h.clear_calls();

    w.show();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_visible=true"});
}

TEST_CASE("window::close fires closed signal and toggles is_visible",
          "[mock][window]") {
    internal::basic_window w;
    internal::window_handler<platform::mock> h;
    h.map_is_visible(w);

    int closed_count = 0;
    signal_slot<> closed_slot;
    struct closed_cb_t {
        int* count;
        void operator()() const { ++*count; }
    } closed_cb{&closed_count};
    w.closed.subscribe(closed_slot, closed_cb);

    w.show();
    h.clear_calls();
    w.close();

    REQUIRE(closed_count == 1);
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_visible=false"});
}

TEST_CASE("window activated simulator drives user-side subscribers",
          "[mock][window]") {
    internal::basic_window w;
    internal::window_handler<platform::mock> h;

    int activated_count = 0;
    signal_slot<> activated_slot;
    struct activated_cb_t {
        int* count;
        void operator()() const { ++*count; }
    } activated_cb{&activated_count};
    w.activated.subscribe(activated_slot, activated_cb);

    h.simulate_activated(w);
    h.simulate_activated(w);

    REQUIRE(activated_count == 2);
}
