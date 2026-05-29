// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0005 resource_dictionary +
// style + static_resource walker.

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/resource_recorder.hpp>
#include <mpapp/layout.hpp>
#include <mpapp/resources/resource_dictionary.hpp>
#include <mpapp/resources/static_resource.hpp>
#include <mpapp/resources/style.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

// Concrete `view` subclass for tests — `view` is abstract because its
// dtor is virtual but it is otherwise fully usable as a base. Tests
// stand up small visual trees using these so we can exercise the
// parent-pointer walker without depending on any concrete control's
// wrapper-component initialization.
class test_view : public view {
public:
    test_view() = default;
};

// Tiny test-only layout subclass — `layout` requires a
// `create_layout_manager()` virtual in some configurations but for the
// child-list / parent-pointer plumbing we don't need a real manager.
class test_layout : public layout {
public:
    test_layout() = default;
};

} // namespace

TEST_CASE("resource_dictionary stores + retrieves typed values",
          "[mock][resources]") {
    resource_dictionary d;
    d.put("BrandPrimary",   std::string{ "#264653" });
    d.put("DefaultPadding", 16.0);

    CHECK(d.try_get<std::string>("BrandPrimary") == "#264653");
    CHECK(d.try_get<double>("DefaultPadding")    == 16.0);

    // Wrong type — try_get returns nullopt rather than throwing.
    CHECK_FALSE(d.try_get<double>("BrandPrimary").has_value());

    // Missing key.
    CHECK_FALSE(d.try_get<std::string>("Missing").has_value());

    // has() reflects only local entries.
    CHECK(d.has("BrandPrimary"));
    CHECK_FALSE(d.has("Missing"));
}

TEST_CASE("resource_dictionary put on existing key replaces + emits changed",
          "[mock][resources]") {
    resource_dictionary d;
    resource_recorder   rec{ d };

    d.put("AccentColor", std::string{ "#E76F51" });
    d.put("AccentColor", std::string{ "#2A9D8F" });   // override

    CHECK(d.try_get<std::string>("AccentColor") == "#2A9D8F");

    // Recorder captured both puts via the changed signal.
    REQUIRE(rec.calls_as_strings() == std::vector<std::string>{
        "put=AccentColor",
        "put=AccentColor",
    });
}

TEST_CASE("resource_dictionary remove fires changed with null new_value",
          "[mock][resources]") {
    resource_dictionary d;
    resource_recorder   rec{ d };

    d.put("Temp", 1.0);
    d.remove("Temp");
    d.remove("NeverThere");          // no-op — must not emit

    CHECK_FALSE(d.has("Temp"));
    REQUIRE(rec.calls_as_strings() == std::vector<std::string>{
        "put=Temp",
        "remove=Temp",
    });
}

TEST_CASE("resource_dictionary try_get walks merged_dictionaries in order",
          "[mock][resources]") {
    auto theme_dark  = std::make_shared<resource_dictionary>();
    theme_dark->put("BgColor",     std::string{ "#1F1F1F" });
    theme_dark->put("Accent",      std::string{ "#2A9D8F" });

    auto theme_brand = std::make_shared<resource_dictionary>();
    theme_brand->put("BgColor",    std::string{ "#264653" }); // wins for BgColor lookups
    theme_brand->put("BrandColor", std::string{ "#E76F51" });

    resource_dictionary root;
    root.put("Padding", 12.0);
    root.add_merged_dictionary(theme_brand);   // first → wins on duplicate keys
    root.add_merged_dictionary(theme_dark);

    // Local store wins over merged.
    CHECK(root.try_get<double>("Padding") == 12.0);

    // First merged dictionary wins on duplicate keys.
    CHECK(root.try_get<std::string>("BgColor")    == "#264653");
    // Brand-only key resolves from theme_brand.
    CHECK(root.try_get<std::string>("BrandColor") == "#E76F51");
    // Dark-only key resolves by falling through theme_brand to theme_dark.
    CHECK(root.try_get<std::string>("Accent")     == "#2A9D8F");

    // Missing in both still nullopt.
    CHECK_FALSE(root.try_get<std::string>("NotPresent").has_value());
}

TEST_CASE("resource_dictionary composition_changed fires on merged-dict mutation",
          "[mock][resources]") {
    resource_dictionary root;
    resource_recorder   rec{ root };

    auto dark = std::make_shared<resource_dictionary>();
    dark->put("BgColor", std::string{ "#1F1F1F" });

    root.add_merged_dictionary(dark);
    root.clear_merged_dictionaries();

    REQUIRE(rec.calls_as_strings() == std::vector<std::string>{
        "composition_changed",
        "composition_changed",
    });
}

TEST_CASE("find_in walks view -> parent -> ... and returns first typed match",
          "[mock][resources][static_resource]") {
    // root.resources = { BrandColor }
    // mid is a layout; its resources = { Accent }
    // leaf is a child view with no local resources
    test_layout root;
    root.resources = std::make_shared<resource_dictionary>();
    root.resources->put("BrandColor", std::string{ "#E76F51" });
    root.resources->put("Padding",    12.0);

    test_layout mid;
    mid.resources = std::make_shared<resource_dictionary>();
    mid.resources->put("Accent",      std::string{ "#2A9D8F" });
    // Padding shadowing — mid wins for closer-ancestor lookups.
    mid.resources->put("Padding",     8.0);
    root.add(mid);

    test_view leaf;
    mid.add(leaf);

    // Local-or-ancestor key resolves at the nearest ancestor with the key.
    CHECK(find_in<std::string>(leaf, "Accent")     == "#2A9D8F");
    CHECK(find_in<std::string>(leaf, "BrandColor") == "#E76F51");

    // Nearest ancestor wins on duplicate keys (mid overrides root).
    CHECK(find_in<double>(leaf, "Padding")         == 8.0);

    // Wrong type at one level falls through to the next ancestor that
    // has the right-typed entry, then nullopt.
    CHECK_FALSE(find_in<int>(leaf, "Padding").has_value());

    // Missing in the entire chain — nullopt.
    CHECK_FALSE(find_in<std::string>(leaf, "Missing").has_value());
}

TEST_CASE("find_in tolerates null per-view resources gracefully",
          "[mock][resources][static_resource]") {
    test_layout root;
    // root has NO local resources.
    test_view leaf;
    root.add(leaf);

    CHECK_FALSE(find_in<std::string>(leaf, "Whatever").has_value());

    // After attaching one, the walk picks it up.
    root.resources = std::make_shared<resource_dictionary>();
    root.resources->put("Hello", std::string{ "world" });
    CHECK(find_in<std::string>(leaf, "Hello") == "world");
}

TEST_CASE("layout::remove + clear detach the child's parent pointer",
          "[mock][resources][layout]") {
    test_layout root;
    test_view a;
    test_view b;

    root.add(a);
    root.add(b);
    REQUIRE(a.parent() == &root);
    REQUIRE(b.parent() == &root);

    root.remove(a);
    CHECK(a.parent() == nullptr);
    CHECK(b.parent() == &root);

    root.clear();
    CHECK(b.parent() == nullptr);
}

TEST_CASE("style.apply_to runs every setter against the target view",
          "[mock][resources][style]") {
    style btn_style{ "Button" };

    int calls_a = 0;
    int calls_b = 0;
    btn_style.setters["a"] = [&calls_a](view&) { ++calls_a; };
    btn_style.setters["b"] = [&calls_b](view&) { ++calls_b; };

    test_view v;
    btn_style.apply_to(v);

    CHECK(calls_a == 1);
    CHECK(calls_b == 1);
}

TEST_CASE("style.based_on runs parent setters before this style's setters",
          "[mock][resources][style]") {
    auto base = std::make_shared<style>("View");
    std::vector<std::string> order;
    base->setters["base_only"]  = [&order](view&) { order.emplace_back("base.base_only"); };
    base->setters["overridden"] = [&order](view&) { order.emplace_back("base.overridden"); };

    style derived{ "Button" };
    derived.based_on = base;
    derived.setters["overridden"] = [&order](view&) { order.emplace_back("derived.overridden"); };
    derived.setters["derived_only"] = [&order](view&) { order.emplace_back("derived.derived_only"); };

    test_view v;
    derived.apply_to(v);

    // base.* runs before derived.* (so derived wins on duplicate keys).
    // Within a single style the iteration order is unordered_map's —
    // we only assert that the base's entries precede the derived's, not
    // the relative order of entries within a single map.
    REQUIRE(order.size() == 4);
    auto idx_of = [&](const std::string& key) {
        for (std::size_t i = 0; i < order.size(); ++i) {
            if (order[i] == key) {
                return static_cast<std::ptrdiff_t>(i);
            }
        }
        return std::ptrdiff_t{ -1 };
    };
    CHECK(idx_of("base.base_only")     < idx_of("derived.overridden"));
    CHECK(idx_of("base.overridden")    < idx_of("derived.overridden"));
    CHECK(idx_of("base.base_only")     < idx_of("derived.derived_only"));
    CHECK(idx_of("base.overridden")    < idx_of("derived.derived_only"));
}

TEST_CASE("style.apply_to swallows setter exceptions per RFC-0005",
          "[mock][resources][style]") {
    style s{ "View" };
    bool  ran_after_throw = false;
    s.setters["bad"]  = [](view&) { throw std::runtime_error("oops"); };
    s.setters["good"] = [&ran_after_throw](view&) { ran_after_throw = true; };

    test_view v;
    REQUIRE_NOTHROW(s.apply_to(v));
    CHECK(ran_after_throw);
}

TEST_CASE("resource_recorder.try_get reports hit + miss + returns the value",
          "[mock][resources][recorder]") {
    resource_dictionary d;
    d.put("Color", std::string{ "#264653" });

    resource_recorder rec{ d };
    rec.clear_calls();   // drop the noise the constructor's subscription
                         // would have produced if any setup recorded.

    auto hit  = rec.try_get<std::string>("Color");
    auto miss = rec.try_get<std::string>("AccentColor");

    CHECK(hit  == "#264653");
    CHECK_FALSE(miss.has_value());

    REQUIRE(rec.calls_as_strings() == std::vector<std::string>{
        "lookup.hit=Color",
        "lookup.miss=AccentColor",
    });
}
