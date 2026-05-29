// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::pinch_gesture_recognizer`
// per [[vault/30_RFCs/RFC-0003-gesture-recognizers]].

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_button.hpp>
#include <mpapp/gestures/pinch_gesture_recognizer.hpp>
#include <mpapp/handlers/mock/view_handler.hpp>

using namespace mpapp;

TEST_CASE("pinch_gesture_recognizer attaches with MAUI defaults",
          "[mock][gesture][pinch]") {
    internal::basic_button b;
    auto& pinch = b.add_gesture<pinch_gesture_recognizer>();

    REQUIRE(b.gesture_recognizers.size() == 1);
    CHECK(pinch.kind() == internal::gesture_kind::pinch);
}

TEST_CASE("mock view_handler records gesture.pinch_attached",
          "[mock][gesture][pinch]") {
    internal::basic_button b;
    b.add_gesture<pinch_gesture_recognizer>();

    view_handler<platform::mock> h;
    h.map_gestures(b);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "gesture.pinch_attached",
    });
}

TEST_CASE("simulate_pinch carries scale + normalised origin",
          "[mock][gesture][pinch]") {
    internal::basic_button b;
    auto& pinch = b.add_gesture<pinch_gesture_recognizer>();

    std::vector<pinch_updated_event_args> seen;
    signal_slot<const pinch_updated_event_args&> slot;
    struct cb_t {
        std::vector<pinch_updated_event_args>* out;
        void operator()(const pinch_updated_event_args& a) const { out->push_back(a); }
    } cb{&seen};
    pinch.pinch_updated.subscribe(slot, cb);

    view_handler<platform::mock> h;
    // Zoom in 1.5x at 25% from top-left.
    h.simulate_pinch(b, internal::gesture_status::started,   1.0, 0.25, 0.25);
    h.simulate_pinch(b, internal::gesture_status::running,   1.5, 0.25, 0.25);
    h.simulate_pinch(b, internal::gesture_status::completed, 1.0, 0.25, 0.25);

    REQUIRE(seen.size() == 3);
    CHECK(seen[1].scale    == 1.5);
    CHECK(seen[1].origin_x == 0.25);
    CHECK(seen[1].origin_y == 0.25);
    CHECK(seen[2].status   == internal::gesture_status::completed);
}
