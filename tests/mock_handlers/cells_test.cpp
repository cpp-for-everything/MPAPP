// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for the TableView cell-type tree (ADR-0021).

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/cell.hpp>
#include <mpapp/internal/basic_entry_cell.hpp>
#include <mpapp/handlers/mock/entry_cell_handler.hpp>
#include <mpapp/handlers/mock/image_cell_handler.hpp>
#include <mpapp/handlers/mock/switch_cell_handler.hpp>
#include <mpapp/handlers/mock/text_cell_handler.hpp>
#include <mpapp/handlers/mock/view_cell_handler.hpp>
#include <mpapp/internal/basic_image_cell.hpp>
#include <mpapp/internal/basic_label.hpp>
#include <mpapp/internal/basic_switch_cell.hpp>
#include <mpapp/internal/basic_text_cell.hpp>
#include <mpapp/internal/basic_view_cell.hpp>

using namespace mpapp;

TEST_CASE("text_cell mock records text + detail changes",
          "[mock][cell][text_cell]") {
    internal::basic_text_cell c;
    internal::text_cell_handler<platform::mock> h;
    h.map_text(c);
    h.map_detail(c);
    h.clear_calls();

    c.text   = "Sign out";
    c.detail = "Last login: today";

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "text=Sign out",
        "detail=Last login: today",
    });
}

TEST_CASE("entry_cell mock records label + text changes",
          "[mock][cell][entry_cell]") {
    internal::basic_entry_cell c;
    internal::entry_cell_handler<platform::mock> h;
    h.map_label(c);
    h.map_text(c);
    h.clear_calls();

    c.label = "Email";
    c.text  = "user@example.com";

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "label=Email",
        "text=user@example.com",
    });
}

TEST_CASE("switch_cell toggle() flips on + emits signal",
          "[mock][cell][switch_cell]") {
    internal::basic_switch_cell c;
    internal::switch_cell_handler<platform::mock> h;
    h.map_text(c);
    h.map_on(c);
    h.clear_calls();

    int hits = 0;
    bool last = false;
    struct cb_t { int* hits; bool* last;
                  void operator()(bool v) const { ++*hits; *last = v; } };
    cb_t cb{&hits, &last};
    signal_slot<bool> slot{};
    c.on_changed.subscribe(slot, cb);

    c.text = "Notifications";
    c.toggle();
    c.toggle();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "text=Notifications",
        "on=true",
        "on=false",
    });
    CHECK(hits == 2);
    CHECK(last == false);
}

TEST_CASE("view_cell records content.present transitions",
          "[mock][cell][view_cell]") {
    internal::basic_label child;
    internal::basic_view_cell c;
    internal::view_cell_handler<platform::mock> h;
    h.map_content(c);
    h.clear_calls();

    c.content = &child;
    c.content = nullptr;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "content.present=true",
        "content.present=false",
    });
}

TEST_CASE("image_cell carries text + image_uri",
          "[mock][cell][image_cell]") {
    internal::basic_image_cell c;
    internal::image_cell_handler<platform::mock> h;
    h.map_text(c);
    h.map_image_uri(c);
    h.clear_calls();

    c.text      = "Profile";
    c.image_uri = "file:///profile.png";

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "text=Profile",
        "image_uri=file:///profile.png",
    });
}

TEST_CASE("image_cell inherits text_cell so detail also exists",
          "[mock][cell][image_cell]") {
    internal::basic_image_cell c;
    c.text      = "Account";
    c.detail    = "user@example.com";
    c.image_uri = "file:///avatar.png";
    // Just verifying the inherited surface compiles + assigns; the
    // mock handler above only maps text + image_uri.
    CHECK(c.text.get()      == "Account");
    CHECK(c.detail.get()    == "user@example.com");
    CHECK(c.image_uri.get() == "file:///avatar.png");
}

TEST_CASE("cell base carries is_enabled + tapped",
          "[mock][cell]") {
    internal::basic_text_cell c;
    CHECK(c.is_enabled.get() == true);

    int hits = 0;
    struct cb_t { int* hits; void operator()() const { ++*hits; } };
    cb_t cb{&hits};
    signal_slot<> slot{};
    c.tapped.subscribe(slot, cb);
    c.tapped.emit();
    c.tapped.emit();
    CHECK(hits == 2);

    c.is_enabled = false;
    CHECK(c.is_enabled.get() == false);
}
