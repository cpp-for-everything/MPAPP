// SPDX-License-Identifier: Apache-2.0
// T-0002 — Template wrapper type spike.
//
// These tests exercise Observable<T>, Computed<...>, Command<>, and the
// intrusive signal/slot from include/mpapp/. They also instantiate the
// example view-model from ADR-0009 to prove that user-facing surface
// compiles unchanged.

#include <catch2/catch_test_macros.hpp>

#include <format>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

// This spike exercises only the platform-neutral template-wrapper
// primitives — it never instantiates a wrapper component, so it includes
// the specific primitive headers rather than the app umbrella
// <mpapp/mpapp.hpp>. The umbrella drags in every wrapper, and each
// wrapper embeds a <platform::current> handler (WinUI/GTK4/…) that this
// pure-surface test must not depend on (T-0032 Path B — keep core-side
// test TUs free of the platform SDK).
#include <mpapp/command.hpp>
#include <mpapp/computed.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/signal.hpp>

// --------------------------------------------------------------------------
// View model used across multiple test cases. Mirrors the example in
// ADR-0009 § Option A so we exercise the same surface a real user would.
// --------------------------------------------------------------------------
struct todo_view_model {
    mpapp::Observable<int>         count{0};
    mpapp::Observable<std::string> name{"Alice"};

    auto display(mpapp::Computed<&todo_view_model::count,
                                 &todo_view_model::name> = {}) const {
        return std::format("{}: {}", name.get(), count.get());
    }

    void increment(mpapp::Command<> = {}) {
        count.set(count.get() + 1);
    }
};

// --------------------------------------------------------------------------
// Tiny helper that captures call counts and the most recent value emitted
// by an Observable<int>::changed signal, without dragging in std::function.
// --------------------------------------------------------------------------
struct probe {
    int last_value_seen{};
    int call_count{0};

    void operator()(int v) {
        last_value_seen = v;
        ++call_count;
    }
};

// ==========================================================================
// 1. Observable<int>: set changes value & fires once; same-value set is a
//    no-op.
// ==========================================================================
TEST_CASE("Observable<int> set fires once and is idempotent on same value",
          "[observable]") {
    mpapp::Observable<int> count{0};

    probe p;
    mpapp::signal_slot<const int&> slot;
    count.changed.subscribe(slot, p);

    REQUIRE(count.get() == 0);
    REQUIRE(p.call_count == 0);

    count.set(1);
    REQUIRE(count.get() == 1);
    REQUIRE(p.call_count == 1);
    REQUIRE(p.last_value_seen == 1);

    count.set(1); // same value — must not fire
    REQUIRE(p.call_count == 1);

    count = 42; // operator= path
    REQUIRE(count.get() == 42);
    REQUIRE(p.call_count == 2);
    REQUIRE(p.last_value_seen == 42);
}

TEST_CASE("Observable: slot disconnects on destruction (auto-unsubscribe)",
          "[observable][signal]") {
    mpapp::Observable<int> n{0};
    probe p;

    {
        mpapp::signal_slot<const int&> slot;
        n.changed.subscribe(slot, p);
        n.set(1);
        REQUIRE(p.call_count == 1);
    } // slot destructor runs

    REQUIRE(n.changed.subscriber_count() == 0);
    n.set(2);
    REQUIRE(p.call_count == 1); // not invoked again
}

TEST_CASE("Observable: explicit disconnect stops further notifications",
          "[observable][signal]") {
    mpapp::Observable<int> n{0};
    probe p;
    mpapp::signal_slot<const int&> slot;

    n.changed.subscribe(slot, p);
    n.set(1);
    REQUIRE(p.call_count == 1);

    slot.disconnect();
    REQUIRE_FALSE(slot.connected());
    n.set(2);
    REQUIRE(p.call_count == 1);
}

TEST_CASE("Observable: multiple subscribers all fire", "[observable][signal]") {
    mpapp::Observable<int> n{0};
    probe a;
    probe b;
    mpapp::signal_slot<const int&> slot_a;
    mpapp::signal_slot<const int&> slot_b;

    n.changed.subscribe(slot_a, a);
    n.changed.subscribe(slot_b, b);
    REQUIRE(n.changed.subscriber_count() == 2);

    n.set(7);
    REQUIRE(a.call_count == 1);
    REQUIRE(b.call_count == 1);
    REQUIRE(a.last_value_seen == 7);
    REQUIRE(b.last_value_seen == 7);
}

// ==========================================================================
// 2. Computed<&VM::a, &VM::b>: re-evaluates when a OR b changes.
//
//    For the spike we do not wire a fully generic dependency tracker —
//    instead, we subscribe a re-evaluator manually using the dependency
//    list exposed by `computed_traits`, then verify that the count of
//    re-evaluations matches the count of dependency changes.
// ==========================================================================
namespace {

// Recompute the result of `vm.display()` and store it in `out`. Wired to
// fire whenever any dependency listed in `Computed<&VM::a, &VM::b>` changes.
struct display_recomputer {
    todo_view_model* vm;
    std::string*     out;
    int*             ticks;

    template <class U>
    void operator()(const U&) {
        *out = vm->display();
        ++*ticks;
    }
};

} // namespace

TEST_CASE("Computed<&VM::a, &VM::b> re-evaluates on each dependency change",
          "[computed]") {
    todo_view_model vm;
    std::string current = vm.display();
    REQUIRE(current == "Alice: 0");

    int ticks = 0;
    display_recomputer recomputer{&vm, &current, &ticks};

    mpapp::signal_slot<const int&>         count_slot;
    mpapp::signal_slot<const std::string&> name_slot;
    vm.count.changed.subscribe(count_slot, recomputer);
    vm.name.changed.subscribe(name_slot, recomputer);

    // Dependency 1 changes.
    vm.count.set(5);
    REQUIRE(ticks == 1);
    REQUIRE(current == "Alice: 5");

    // Dependency 2 changes.
    vm.name.set("Bob");
    REQUIRE(ticks == 2);
    REQUIRE(current == "Bob: 5");

    // No change → no re-evaluation.
    vm.count.set(5);
    REQUIRE(ticks == 2);
}

TEST_CASE("Computed tag is detected by the is_computed_tag concept",
          "[computed][concept]") {
    using Cm = mpapp::Computed<&todo_view_model::count,
                               &todo_view_model::name>;
    STATIC_REQUIRE(mpapp::is_computed_tag<Cm>);
    STATIC_REQUIRE(mpapp::is_computed_tag<const Cm&>);
    STATIC_REQUIRE_FALSE(mpapp::is_computed_tag<int>);
    STATIC_REQUIRE_FALSE(mpapp::is_computed_tag<todo_view_model>);

    STATIC_REQUIRE(mpapp::computed_traits<Cm>::count == 2);
}

// ==========================================================================
// 3. Command<>: presence on a method is detectable via a concept.
// ==========================================================================
namespace {

struct method_probe {
    void increment(mpapp::Command<> = {}) {}
    void rename(std::string, mpapp::Command<std::string> = {}) {}
    void other() {}
    int  also_other(int x) const { return x; }
};

} // namespace

TEST_CASE("is_command_method detects Command<> in the method signature",
          "[command][concept]") {
    STATIC_REQUIRE(mpapp::is_command_method<&method_probe::increment>);
    STATIC_REQUIRE(mpapp::is_command_method<&method_probe::rename>);
    STATIC_REQUIRE_FALSE(mpapp::is_command_method<&method_probe::other>);
    STATIC_REQUIRE_FALSE(mpapp::is_command_method<&method_probe::also_other>);

    // Tag-type detection on the parameter type itself.
    STATIC_REQUIRE(mpapp::is_command_tag<mpapp::Command<>>);
    STATIC_REQUIRE(mpapp::is_command_tag<mpapp::Command<int, std::string>>);
    STATIC_REQUIRE_FALSE(mpapp::is_command_tag<int>);

    STATIC_REQUIRE(mpapp::command_traits<mpapp::Command<>>::count == 0);
    STATIC_REQUIRE(
        mpapp::command_traits<mpapp::Command<int, std::string>>::count == 2);
}

TEST_CASE("Command<> method is callable normally", "[command]") {
    todo_view_model vm;
    REQUIRE(vm.count.get() == 0);
    vm.increment();
    REQUIRE(vm.count.get() == 1);
    vm.increment();
    REQUIRE(vm.count.get() == 2);
}

// ==========================================================================
// 4. Move-only T works inside Observable<T>.
// ==========================================================================
TEST_CASE("Observable<move-only T> compiles and behaves", "[observable][move]") {
    using uptr = std::unique_ptr<int>;
    mpapp::Observable<uptr> o;
    REQUIRE(o.get() == nullptr);

    int hit_count = 0;
    auto callback = [&hit_count](const uptr&) { ++hit_count; };
    mpapp::signal_slot<const uptr&> slot;
    o.changed.subscribe(slot, callback);

    o.set(std::make_unique<int>(42));
    REQUIRE(o.get() != nullptr);
    REQUIRE(*o.get() == 42);
    REQUIRE(hit_count == 1);

    // Move-only T has no operator==, so the change-on-real-change short
    // circuit degrades to always-change — verify a second assignment fires.
    o.set(std::make_unique<int>(7));
    REQUIRE(*o.get() == 7);
    REQUIRE(hit_count == 2);
}

// ==========================================================================
// 5. Observable<std::string>: implicit conversion to const std::string&.
// ==========================================================================
TEST_CASE("Observable<std::string> implicitly converts to const string&",
          "[observable][string]") {
    mpapp::Observable<std::string> s{"hello"};

    // Implicit conversion site: binding to a const std::string& parameter.
    auto take = [](const std::string& v) { return v.size(); };
    REQUIRE(take(s) == 5);

    // Implicit conversion site: direct initialisation.
    const std::string& ref = s;
    REQUIRE(ref == "hello");

    // operator= via T overload.
    s = std::string{"world!"};
    REQUIRE(s.get() == "world!");
    REQUIRE(take(s) == 6);
}

// ==========================================================================
// 6. End-to-end: the ADR-0009 example compiles and behaves.
//    (The todo_view_model declaration at the top of this file IS that
//    example verbatim. We exercise both `display` and `increment` here.)
// ==========================================================================
TEST_CASE("ADR-0009 todo_view_model example compiles and behaves",
          "[adr-0009][e2e]") {
    todo_view_model vm;
    REQUIRE(vm.display() == "Alice: 0");
    vm.increment();
    REQUIRE(vm.display() == "Alice: 1");
    vm.name.set("Bob");
    REQUIRE(vm.display() == "Bob: 1");
}
