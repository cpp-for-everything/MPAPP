// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::pan_gesture_recognizer`
// per [[vault/30_RFCs/RFC-0003-gesture-recognizers]].

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/button.hpp>
#include <mpapp/gestures/pan_gesture_recognizer.hpp>
#include <mpapp/handlers/mock/view_handler.hpp>

using namespace mpapp;

TEST_CASE("pan_gesture_recognizer attaches with MAUI defaults",
          "[mock][gesture][pan]") {
    internal::basic_button b;
    auto& pan = b.add_gesture<pan_gesture_recognizer>();

    REQUIRE(b.gesture_recognizers.size() == 1);
    CHECK(pan.kind() == internal::gesture_kind::pan);
    CHECK(pan.touch_points.get() == 1);
}

TEST_CASE("mock view_handler records gesture.pan_attached",
          "[mock][gesture][pan]") {
    internal::basic_button b;
    b.add_gesture<pan_gesture_recognizer>();

    view_handler<platform::mock> h;
    h.map_gestures(b);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "gesture.pan_attached",
    });
}

TEST_CASE("simulate_pan drives the started -> running -> completed lifecycle",
          "[mock][gesture][pan]") {
    internal::basic_button b;
    auto& pan = b.add_gesture<pan_gesture_recognizer>();

    std::vector<pan_updated_event_args> seen;
    signal_slot<const pan_updated_event_args&> slot;
    struct cb_t {
        std::vector<pan_updated_event_args>* out;
        void operator()(const pan_updated_event_args& a) const { out->push_back(a); }
    } cb{&seen};
    pan.pan_updated.subscribe(slot, cb);

    view_handler<platform::mock> h;
    h.simulate_pan(b, internal::gesture_status::started,   1, 0.0, 0.0);
    h.simulate_pan(b, internal::gesture_status::running,   1, 5.0, 0.0);
    h.simulate_pan(b, internal::gesture_status::running,   1, 12.0, -3.0);
    h.simulate_pan(b, internal::gesture_status::completed, 1, 12.0, -3.0);

    REQUIRE(seen.size() == 4);
    CHECK(seen[0].status == internal::gesture_status::started);
    CHECK(seen[1].status == internal::gesture_status::running);
    CHECK(seen[1].total_x == 5.0);
    CHECK(seen[2].total_x == 12.0);
    CHECK(seen[2].total_y == -3.0);
    CHECK(seen[3].status == internal::gesture_status::completed);
    // gesture_id is sticky for the same gesture stream.
    for (const auto& a : seen) CHECK(a.gesture_id == 1);
}
