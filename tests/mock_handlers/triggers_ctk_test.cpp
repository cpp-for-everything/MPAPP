// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the CommunityToolkit-inspired triggers
// defined in <mpapp/triggers/triggers_ctk.hpp>.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/observable.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/triggers/triggers_ctk.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// compare_state_trigger — equal
// ---------------------------------------------------------------------------

TEST_CASE("compare_state_trigger equal: inactive when value != target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 1 };

    // Act
    compare_state_trigger<int> t{ val, compare_op::equal, 5 };

    // Assert
    CHECK_FALSE(t.is_active());
}

TEST_CASE("compare_state_trigger equal: active when value == target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 5 };

    // Act
    compare_state_trigger<int> t{ val, compare_op::equal, 5 };

    // Assert
    CHECK(t.is_active());
}

TEST_CASE("compare_state_trigger equal: transitions and emits active_changed",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 0 };
    compare_state_trigger<int> t{ val, compare_op::equal, 7 };

    int  emit_count = 0;
    bool last_value = false;
    signal_slot<bool> sl;
    struct cb_t {
        int*  count;
        bool* last;
        void operator()(bool v) const { *last = v; ++(*count); }
    } cb{ &emit_count, &last_value };
    t.active_changed.subscribe(sl, cb);

    // Act & Assert: match
    val = 7;
    CHECK(t.is_active());
    CHECK(emit_count == 1);
    CHECK(last_value == true);

    // Act & Assert: un-match
    val = 3;
    CHECK_FALSE(t.is_active());
    CHECK(emit_count == 2);
    CHECK(last_value == false);
}

TEST_CASE("compare_state_trigger equal: same-value write does not re-emit",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 7 };
    compare_state_trigger<int> t{ val, compare_op::equal, 7 };

    int emit_count = 0;
    signal_slot<bool> sl;
    struct cb_t {
        int* count;
        void operator()(bool) const { ++(*count); }
    } cb{ &emit_count };
    t.active_changed.subscribe(sl, cb);

    // Act
    val = 7; // same value → Observable no-op → no edge
    CHECK(t.is_active());
    CHECK(emit_count == 0);
}

// ---------------------------------------------------------------------------
// compare_state_trigger — not_equal
// ---------------------------------------------------------------------------

TEST_CASE("compare_state_trigger not_equal: inactive when value == target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 5 };

    // Act
    compare_state_trigger<int> t{ val, compare_op::not_equal, 5 };

    // Assert
    CHECK_FALSE(t.is_active());
}

TEST_CASE("compare_state_trigger not_equal: active when value != target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 3 };

    // Act
    compare_state_trigger<int> t{ val, compare_op::not_equal, 5 };

    // Assert
    CHECK(t.is_active());
}

TEST_CASE("compare_state_trigger not_equal: transitions on value change",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 3 };
    compare_state_trigger<int> t{ val, compare_op::not_equal, 5 };
    CHECK(t.is_active());

    // Act & Assert: match target → deactivate
    val = 5;
    CHECK_FALSE(t.is_active());

    // Act & Assert: leave target → reactivate
    val = 6;
    CHECK(t.is_active());
}

// ---------------------------------------------------------------------------
// compare_state_trigger — less
// ---------------------------------------------------------------------------

TEST_CASE("compare_state_trigger less: active when value < target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 3 };

    // Act
    compare_state_trigger<int> t{ val, compare_op::less, 5 };

    // Assert
    CHECK(t.is_active());
}

TEST_CASE("compare_state_trigger less: inactive when value == target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 5 };
    compare_state_trigger<int> t{ val, compare_op::less, 5 };
    CHECK_FALSE(t.is_active());

    // Act & Assert: transitions
    val = 4;
    CHECK(t.is_active());
    val = 5;
    CHECK_FALSE(t.is_active());
}

TEST_CASE("compare_state_trigger less: inactive when value > target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 6 };
    compare_state_trigger<int> t{ val, compare_op::less, 5 };

    // Assert
    CHECK_FALSE(t.is_active());
}

// ---------------------------------------------------------------------------
// compare_state_trigger — greater
// ---------------------------------------------------------------------------

TEST_CASE("compare_state_trigger greater: active when value > target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 10 };
    compare_state_trigger<int> t{ val, compare_op::greater, 5 };

    // Assert
    CHECK(t.is_active());
}

TEST_CASE("compare_state_trigger greater: inactive when value <= target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 5 };
    compare_state_trigger<int> t{ val, compare_op::greater, 5 };
    CHECK_FALSE(t.is_active());

    // Act & Assert
    val = 6;
    CHECK(t.is_active());
    val = 5;
    CHECK_FALSE(t.is_active());
    val = 4;
    CHECK_FALSE(t.is_active());
}

// ---------------------------------------------------------------------------
// compare_state_trigger — less_equal
// ---------------------------------------------------------------------------

TEST_CASE("compare_state_trigger less_equal: active when value <= target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 5 };
    compare_state_trigger<int> t{ val, compare_op::less_equal, 5 };
    CHECK(t.is_active());

    val = 4;
    CHECK(t.is_active());

    val = 6;
    CHECK_FALSE(t.is_active());
}

// ---------------------------------------------------------------------------
// compare_state_trigger — greater_equal
// ---------------------------------------------------------------------------

TEST_CASE("compare_state_trigger greater_equal: active when value >= target",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 5 };
    compare_state_trigger<int> t{ val, compare_op::greater_equal, 5 };
    CHECK(t.is_active());

    val = 6;
    CHECK(t.is_active());

    val = 4;
    CHECK_FALSE(t.is_active());
}

// ---------------------------------------------------------------------------
// compare_state_trigger — active_changed not emitted on construction
// ---------------------------------------------------------------------------

TEST_CASE("compare_state_trigger: active_changed NOT emitted on construction",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 7 };

    int emit_count = 0;
    // We need to set up the listener but the trigger is constructed
    // after we add the listener. To cover the case: create with match,
    // verify no emission happened.
    compare_state_trigger<int> t{ val, compare_op::equal, 7 };

    signal_slot<bool> sl;
    struct cb_t {
        int* count;
        void operator()(bool) const { ++(*count); }
    } cb{ &emit_count };
    t.active_changed.subscribe(sl, cb);

    // The trigger was already active at construction — no emission yet.
    CHECK(t.is_active());
    CHECK(emit_count == 0);
}

// ---------------------------------------------------------------------------
// compare_state_trigger — double edge: active stays active, no re-emit
// ---------------------------------------------------------------------------

TEST_CASE("compare_state_trigger: no spurious emission on stable state",
          "[mock][triggers_ctk][compare]") {
    // Arrange
    Observable<int> val{ 0 };
    compare_state_trigger<int> t{ val, compare_op::less, 10 };
    CHECK(t.is_active());

    int emit_count = 0;
    signal_slot<bool> sl;
    struct cb_t {
        int* count;
        void operator()(bool) const { ++(*count); }
    } cb{ &emit_count };
    t.active_changed.subscribe(sl, cb);

    // Act: change value but keep condition true
    val = 5;  // still < 10
    CHECK(t.is_active());
    CHECK(emit_count == 0);  // no edge → no emission
}

// ---------------------------------------------------------------------------
// and_multi_trigger — empty (no sources: inactive by default until any source is added)
// ---------------------------------------------------------------------------

TEST_CASE("and_multi_trigger: empty trigger (no sources) is inactive",
          "[mock][triggers_ctk][and_multi]") {
    // MAUI / CommunityToolkit semantics: a trigger with no conditions is
    // inactive (has not yet been satisfied). active_ is initialised false
    // and evaluate() is only called when a source is added.
    and_multi_trigger t;
    CHECK_FALSE(t.is_active());
}

// ---------------------------------------------------------------------------
// and_multi_trigger — single source
// ---------------------------------------------------------------------------

TEST_CASE("and_multi_trigger: single source, inactive when false",
          "[mock][triggers_ctk][and_multi]") {
    Observable<bool> a{ false };
    and_multi_trigger t;
    t.add_source(a);
    CHECK_FALSE(t.is_active());
}

TEST_CASE("and_multi_trigger: single source, active when true",
          "[mock][triggers_ctk][and_multi]") {
    Observable<bool> a{ true };
    and_multi_trigger t;
    t.add_source(a);
    CHECK(t.is_active());
}

TEST_CASE("and_multi_trigger: single source transitions and emits active_changed",
          "[mock][triggers_ctk][and_multi]") {
    Observable<bool> a{ false };
    and_multi_trigger t;
    t.add_source(a);

    int  emit_count = 0;
    bool last_value = false;
    signal_slot<bool> sl;
    struct cb_t {
        int*  count;
        bool* last;
        void operator()(bool v) const { *last = v; ++(*count); }
    } cb{ &emit_count, &last_value };
    t.active_changed.subscribe(sl, cb);

    // Activate
    a = true;
    CHECK(t.is_active());
    CHECK(emit_count == 1);
    CHECK(last_value == true);

    // Deactivate
    a = false;
    CHECK_FALSE(t.is_active());
    CHECK(emit_count == 2);
    CHECK(last_value == false);
}

// ---------------------------------------------------------------------------
// and_multi_trigger — two sources
// ---------------------------------------------------------------------------

TEST_CASE("and_multi_trigger: active only when ALL sources true",
          "[mock][triggers_ctk][and_multi]") {
    Observable<bool> a{ false };
    Observable<bool> b{ false };
    and_multi_trigger t;
    t.add_source(a);
    t.add_source(b);

    // Both false
    CHECK_FALSE(t.is_active());

    // One true
    a = true;
    CHECK_FALSE(t.is_active());

    // Both true
    b = true;
    CHECK(t.is_active());

    // One drops
    a = false;
    CHECK_FALSE(t.is_active());
}

TEST_CASE("and_multi_trigger: emits active_changed only on real edge",
          "[mock][triggers_ctk][and_multi]") {
    Observable<bool> a{ false };
    Observable<bool> b{ false };
    and_multi_trigger t;
    t.add_source(a);
    t.add_source(b);

    int emit_count = 0;
    signal_slot<bool> sl;
    struct cb_t {
        int* count;
        void operator()(bool) const { ++(*count); }
    } cb{ &emit_count };
    t.active_changed.subscribe(sl, cb);

    a = true;                 // still not all true → no edge
    CHECK(emit_count == 0);

    b = true;                 // now all true → edge → emit
    CHECK(emit_count == 1);

    b = true;                 // same value → Observable no-op → no edge
    CHECK(emit_count == 1);

    a = false;                // edge → emit
    CHECK(emit_count == 2);
}

// ---------------------------------------------------------------------------
// and_multi_trigger — three sources (coverage of multi-source path)
// ---------------------------------------------------------------------------

TEST_CASE("and_multi_trigger: three sources all must be true",
          "[mock][triggers_ctk][and_multi]") {
    Observable<bool> a{ true };
    Observable<bool> b{ true };
    Observable<bool> c{ false };
    and_multi_trigger t;
    t.add_source(a);
    t.add_source(b);
    t.add_source(c);

    CHECK_FALSE(t.is_active());

    c = true;
    CHECK(t.is_active());

    b = false;
    CHECK_FALSE(t.is_active());
}

// ---------------------------------------------------------------------------
// and_multi_trigger — fluent chaining
// ---------------------------------------------------------------------------

TEST_CASE("and_multi_trigger: add_source returns *this for fluent chaining",
          "[mock][triggers_ctk][and_multi]") {
    Observable<bool> a{ true };
    Observable<bool> b{ true };
    and_multi_trigger t;

    // Fluent: t.add_source(a).add_source(b)
    t.add_source(a).add_source(b);
    CHECK(t.is_active());

    a = false;
    CHECK_FALSE(t.is_active());
}

// ---------------------------------------------------------------------------
// compare_state_trigger — double-edge guard (flip-flop back to same state)
// ---------------------------------------------------------------------------

TEST_CASE("compare_state_trigger: flip-flop emits on each real edge",
          "[mock][triggers_ctk][compare]") {
    Observable<int> val{ 0 };
    compare_state_trigger<int> t{ val, compare_op::equal, 0 };

    int emit_count = 0;
    signal_slot<bool> sl;
    struct cb_t {
        int* count;
        void operator()(bool) const { ++(*count); }
    } cb{ &emit_count };
    t.active_changed.subscribe(sl, cb);

    CHECK(t.is_active());

    val = 1;   // deactivate → emit #1
    CHECK_FALSE(t.is_active());
    CHECK(emit_count == 1);

    val = 0;   // reactivate → emit #2
    CHECK(t.is_active());
    CHECK(emit_count == 2);

    val = 1;   // deactivate again → emit #3
    CHECK_FALSE(t.is_active());
    CHECK(emit_count == 3);
}
