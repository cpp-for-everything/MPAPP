// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0007 data-binding engine.

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/binding.hpp>
#include <mpapp/binding/binding_context.hpp>
#include <mpapp/binding/multi_binding.hpp>
#include <mpapp/binding/relative_source.hpp>
#include <mpapp/handlers/mock/button_handler.hpp>
#include <mpapp/internal/basic_button.hpp>
#include <mpapp/layout.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class test_view : public view {
public:
    test_view() = default;
};

class test_layout : public layout {
public:
    test_layout() = default;
};

struct sample_vm {
    int         x = 0;
    std::string label{};
};

} // namespace

// ---- binding<T> modes ------------------------------------------------------

TEST_CASE("one_way binding: source drives target, not vice versa",
          "[mock][binding]") {
    Observable<std::string> source{ "alpha" };
    Observable<std::string> target{ "" };

    binding<std::string> b{ source, target, binding_mode::one_way };

    CHECK(target.get() == "alpha");          // initial sync
    source = std::string{ "beta" };
    CHECK(target.get() == "beta");           // source -> target

    target = std::string{ "manual" };
    CHECK(source.get() == "beta");           // target -> source is NOT wired
}

TEST_CASE("two_way binding: both directions, no infinite echo",
          "[mock][binding]") {
    Observable<int> source{ 1 };
    Observable<int> target{ 0 };

    binding<int> b{ source, target, binding_mode::two_way };

    CHECK(target.get() == 1);                // initial: source -> target
    source = 5;
    CHECK(target.get() == 5);                // source -> target
    target = 9;
    CHECK(source.get() == 9);                // target -> source
    // The re-entrancy guard + compare-on-write keep this from looping.
    CHECK(target.get() == 9);
}

TEST_CASE("one_time binding: snapshot at construction, no live link",
          "[mock][binding]") {
    Observable<std::string> source{ "snap" };
    Observable<std::string> target{ "" };

    binding<std::string> b{ source, target, binding_mode::one_time };

    CHECK(target.get() == "snap");
    source = std::string{ "changed" };
    CHECK(target.get() == "snap");           // no live subscription
}

TEST_CASE("one_way_to_source binding: target drives source",
          "[mock][binding]") {
    Observable<int> source{ 0 };
    Observable<int> target{ 42 };

    binding<int> b{ source, target, binding_mode::one_way_to_source };

    CHECK(source.get() == 42);               // initial: target -> source
    target = 7;
    CHECK(source.get() == 7);                // target -> source
    source = 100;
    CHECK(target.get() == 7);                // source -> target is NOT wired
}

// ---- converters ------------------------------------------------------------

TEST_CASE("one_way binding with a source->target converter",
          "[mock][binding][converter]") {
    Observable<int>         source{ 3 };
    Observable<std::string> target{ "" };

    binding<int, std::string> b{
        source, target, binding_mode::one_way,
        [](const int& n) { return "n=" + std::to_string(n); }
    };

    CHECK(target.get() == "n=3");
    source = 10;
    CHECK(target.get() == "n=10");
}

TEST_CASE("two_way binding with both converters round-trips",
          "[mock][binding][converter]") {
    Observable<int>         source{ 0 };
    Observable<std::string> target{ "" };

    binding<int, std::string> b{
        source, target, binding_mode::two_way,
        [](const int& n) { return std::to_string(n); },          // to target
        [](const std::string& s) { return std::stoi(s); }         // to source
    };

    CHECK(target.get() == "0");
    source = 5;
    CHECK(target.get() == "5");
    target = std::string{ "12" };
    CHECK(source.get() == 12);
}

// ---- multi_binding ---------------------------------------------------------

TEST_CASE("multi_binding recomputes target when any source changes",
          "[mock][binding][multi]") {
    Observable<std::string> first{ "Ada" };
    Observable<std::string> last{ "Lovelace" };
    Observable<std::string> full{ "" };

    multi_binding<std::string, std::string, std::string> mb{
        full,
        [](const std::string& f, const std::string& l) { return f + " " + l; },
        first, last
    };

    CHECK(full.get() == "Ada Lovelace");     // initial combine
    first = std::string{ "Charles" };
    CHECK(full.get() == "Charles Lovelace");
    last = std::string{ "Babbage" };
    CHECK(full.get() == "Charles Babbage");
}

TEST_CASE("multi_binding combines heterogeneous source types",
          "[mock][binding][multi]") {
    Observable<std::string> name{ "items" };
    Observable<int>         count{ 2 };
    Observable<std::string> caption{ "" };

    multi_binding<std::string, std::string, int> mb{
        caption,
        [](const std::string& n, const int& c) {
            return n + ": " + std::to_string(c);
        },
        name, count
    };

    CHECK(caption.get() == "items: 2");
    count = 9;
    CHECK(caption.get() == "items: 9");
}

// ---- binding_context -------------------------------------------------------

TEST_CASE("binding_context stores + recovers a typed context",
          "[mock][binding][context]") {
    binding_context ctx;
    CHECK_FALSE(ctx.has_value());
    CHECK(ctx.get<sample_vm>() == nullptr);

    auto vm = std::make_shared<sample_vm>();
    vm->x = 7;
    ctx.set(vm);

    CHECK(ctx.has_value());
    auto got = ctx.get<sample_vm>();
    REQUIRE(got != nullptr);
    CHECK(got->x == 7);
    CHECK(ctx.get<int>() == nullptr);        // wrong type -> nullptr

    ctx.clear();
    CHECK_FALSE(ctx.has_value());
}

TEST_CASE("effective_binding_context inherits the nearest ancestor's",
          "[mock][binding][context]") {
    test_layout root;
    test_layout mid;
    test_view   leaf;
    root.add(mid);
    mid.add(leaf);

    auto vm = std::make_shared<sample_vm>();
    vm->label = "root-ctx";
    root.set_binding_context(vm);

    // leaf + mid have no local context -> inherit root's.
    const auto& eff = effective_binding_context(leaf);
    REQUIRE(eff.has_value());
    CHECK(eff.get<sample_vm>()->label == "root-ctx");

    // A closer context shadows the ancestor's.
    auto mid_vm = std::make_shared<sample_vm>();
    mid_vm->label = "mid-ctx";
    mid.set_binding_context(mid_vm);
    CHECK(effective_binding_context(leaf).get<sample_vm>()->label == "mid-ctx");

    // No context anywhere -> empty.
    test_view orphan;
    CHECK_FALSE(effective_binding_context(orphan).has_value());
}

// ---- relative source -------------------------------------------------------

namespace {
class special_layout : public test_layout {};
} // namespace

TEST_CASE("find_ancestor locates the nearest typed ancestor",
          "[mock][binding][relative_source]") {
    special_layout root;
    test_layout    mid;
    test_view      leaf;
    root.add(mid);
    mid.add(leaf);

    CHECK(find_ancestor<special_layout>(leaf) == &root);
    CHECK(find_ancestor<test_layout>(leaf)    == &mid);   // mid is closer; special_layout is-a test_layout but mid wins
    CHECK(find_ancestor<test_view>(leaf)      == nullptr); // no test_view ancestor above leaf
    CHECK(find_ancestor<test_view>(leaf, /*include_self=*/true) == &leaf);
}

TEST_CASE("resolve_relative_source Self returns the element itself",
          "[mock][binding][relative_source]") {
    test_view v;
    CHECK(resolve_relative_source(v, relative_source_mode::self) == &v);
}

// ---- integration: binding drives the Observable -> handler pipeline --------

TEST_CASE("binding drives a bound property through the mock handler",
          "[mock][binding][handler]") {
    sample_vm vm;
    Observable<std::string> title{ "" };

    internal::basic_button                          btn;
    internal::button_handler<platform::mock>        h;
    h.map_text(btn);     // records initial "text=" + subscribes
    h.clear_calls();

    // Bind the VM-side Observable<string> to the button's text. The
    // binding writes through btn.text.set(...), which fires the same
    // mapper a real Windows/Linux/Android handler installed — proving
    // binding is real on every platform via the existing pipeline.
    title = std::string{ "Save" };           // pre-seed the source
    binding<std::string> b{ title, btn.text, binding_mode::one_way };

    title = std::string{ "Saving..." };

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "text=Save",        // initial sync at binding construction
        "text=Saving...",   // source change propagated through the handler
    });
}
