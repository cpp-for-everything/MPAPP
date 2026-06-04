// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::tap_gesture_recognizer`
// per [[vault/30_RFCs/RFC-0003-gesture-recognizers]].
//
// Validates the contract:
//   * `view::add_gesture<tap_gesture_recognizer>()` enrolls a recognizer
//     in the polymorphic `gesture_recognizers` collection.
//   * The mock `view_handler<platform::mock>::map_gestures(view&)` walks
//     that collection and records one `gesture.tap_attached` entry per
//     tap recognizer found, regardless of how many recognizers are
//     stacked on the view.
//   * `simulate_tap(view&)` fans the synthetic event out to every
//     attached tap recognizer's `tapped` signal - multiple recognizers
//     on one view all receive the event.
//   * Observable config (`number_of_taps_required`, `buttons`) is
//     individually mutable per recognizer; the mock layer doesn't
//     gate on it (real platforms do).

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_button.hpp>
#include <mpapp/gestures/tap_gesture_recognizer.hpp>
#include <mpapp/handlers/mock/view_handler.hpp>

using namespace mpapp;

TEST_CASE("tap_gesture_recognizer attaches to a view via add_gesture",
          "[mock][gesture][tap]") {
    internal::basic_button b;
    auto& tap = b.add_gesture<tap_gesture_recognizer>();

    REQUIRE(b.gesture_recognizers.size() == 1);
    CHECK(b.gesture_recognizers.front().get() == &tap);
    CHECK(tap.kind() == internal::gesture_kind::tap);

    // Defaults match MAUI's TapGestureRecognizer.
    CHECK(tap.number_of_taps_required.get() == 1);
    CHECK(tap.buttons.get() == button_mask::primary);
}

TEST_CASE("mock view_handler::map_gestures records one entry per recognizer",
          "[mock][gesture][tap]") {
    internal::basic_button b;
    b.add_gesture<tap_gesture_recognizer>();
    b.add_gesture<tap_gesture_recognizer>();          // a second tap recognizer

    view_handler<platform::mock> h;
    h.map_gestures(b);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "gesture.tap_attached",
        "gesture.tap_attached",
    });
}

TEST_CASE("simulate_tap fires every attached tap recognizer's `tapped` signal",
          "[mock][gesture][tap]") {
    internal::basic_button b;
    auto& tap = b.add_gesture<tap_gesture_recognizer>();

    int hits = 0;
    tapped_event_args last_args{};
    signal_slot<const tapped_event_args&> slot;
    struct cb_t {
        int* hits;
        tapped_event_args* last;
        void operator()(const tapped_event_args& a) const {
            ++*hits;
            *last = a;
        }
    } cb{&hits, &last_args};
    tap.tapped.subscribe(slot, cb);

    view_handler<platform::mock> h;
    h.map_gestures(b);
    h.simulate_tap(b, /*x=*/12.5, /*y=*/7.0,
                   button_mask::primary | button_mask::secondary);

    CHECK(hits == 1);
    CHECK(last_args.x == 12.5);
    CHECK(last_args.y == 7.0);
    CHECK(any(last_args.buttons, button_mask::primary));
    CHECK(any(last_args.buttons, button_mask::secondary));
}

TEST_CASE("simulate_tap fans out to multiple recognizers on the same view",
          "[mock][gesture][tap]") {
    internal::basic_button b;
    auto& single = b.add_gesture<tap_gesture_recognizer>();
    auto& double_tap = b.add_gesture<tap_gesture_recognizer>();
    double_tap.number_of_taps_required = 2;   // mock layer doesn't gate

    int single_hits = 0;
    int double_hits = 0;
    signal_slot<const tapped_event_args&> slot_a;
    signal_slot<const tapped_event_args&> slot_b;
    struct cb_t {
        int* counter;
        void operator()(const tapped_event_args&) const { ++*counter; }
    };
    cb_t cb_single{&single_hits};
    cb_t cb_double{&double_hits};
    single.tapped.subscribe(slot_a, cb_single);
    double_tap.tapped.subscribe(slot_b, cb_double);

    view_handler<platform::mock> h;
    h.simulate_tap(b);
    h.simulate_tap(b);

    CHECK(single_hits == 2);
    CHECK(double_hits == 2);
}

TEST_CASE("Observable<button_mask> emits one change per real value flip",
          "[mock][gesture][tap]") {
    tap_gesture_recognizer tap;

    int hits = 0;
    signal_slot<const button_mask&> slot;
    struct cb_t {
        int* hits;
        void operator()(button_mask) const { ++*hits; }
    } cb{&hits};
    tap.buttons.changed.subscribe(slot, cb);

    tap.buttons = button_mask::secondary;   // change
    tap.buttons = button_mask::secondary;   // suppressed
    tap.buttons = button_mask::primary | button_mask::secondary;  // change

    REQUIRE(hits == 2);
}
