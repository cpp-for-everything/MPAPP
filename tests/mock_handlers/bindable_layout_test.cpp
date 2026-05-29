// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BindableLayout.md
//
// Mock-handler tests for `mpapp::bindable_layout` (attached-property
// facility — CLAUDE Rule 6 / ADR-0008).

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <vector>

#include <mpapp/bindable_layout.hpp>
#include <mpapp/handlers/mock/bindable_layout_handler.hpp>
#include <mpapp/layout.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

// Detaches its bindable_layout attached-property state on destruction.
// bindable_layout keys its static side-table by host POINTER and does
// not auto-clear (see bindable_layout.hpp: callers detach from the
// host dtor). Without this, two stack `test_layout` objects in
// different TEST_CASEs can reuse the same address (common under MSVC)
// and the second test reads the first's stale entries — a real
// test-isolation bug surfaced once the suite started running on
// Windows. Mirrors the documented real-world contract.
class test_layout : public layout {
public:
    ~test_layout() override { bindable_layout::detach(*this); }
};

} // namespace

TEST_CASE("bindable_layout attached properties round-trip",
          "[mock][bindable_layout]") {
    test_layout host;

    items_source items;
    items.items = {"alpha", "beta", "gamma"};
    bindable_layout::set_items_source(host, items);

    CHECK(bindable_layout::get_items_source(host).items.size() == 3);
    CHECK(bindable_layout::get_items_source(host).items[1] == "beta");

    data_template tpl{"label-factory",
                      [](const std::string& s) {
                          (void)s;
                          return std::shared_ptr<view>{};
                      }};
    bindable_layout::set_item_template(host, std::move(tpl));
    CHECK(bindable_layout::get_item_template(host).name == "label-factory");
}

TEST_CASE("bindable_layout mock handler records mapper calls",
          "[mock][bindable_layout]") {
    test_layout host;
    bindable_layout_handler<platform::mock> h;

    // Initial: no items, no template.
    h.map_items_source(host);
    h.map_item_template(host);
    h.map_empty_view(host);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "items_source.count");
    CHECK(h.calls()[0].value_repr    == "0");
    CHECK(h.calls()[1].property_name == "item_template");
    CHECK(h.calls()[1].value_repr.empty());     // default-constructed name
    CHECK(h.calls()[2].property_name == "empty_view.present");
    CHECK(h.calls()[2].value_repr    == "false");
}

TEST_CASE("bindable_layout mock handler reflects mutations on re-mapping",
          "[mock][bindable_layout][sequence]") {
    test_layout host;
    bindable_layout_handler<platform::mock> h;

    bindable_layout::set_items_source(host, items_source{{"x", "y"}});
    h.map_items_source(host);

    bindable_layout::set_items_source(host, items_source{{"x", "y", "z", "w"}});
    h.map_items_source(host);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].value_repr == "2");
    CHECK(h.calls()[1].value_repr == "4");
}

TEST_CASE("bindable_layout::enable sets both properties at once",
          "[mock][bindable_layout]") {
    test_layout host;

    bindable_layout::enable(
        host,
        items_source{{"a", "b"}},
        data_template{"factory", [](const std::string&) {
            return std::shared_ptr<view>{};
        }});

    CHECK(bindable_layout::get_items_source(host).items.size() == 2);
    CHECK(bindable_layout::get_item_template(host).name == "factory");
    CHECK_FALSE(bindable_layout::get_item_template(host).empty());
}
