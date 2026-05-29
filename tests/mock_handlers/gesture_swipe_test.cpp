// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::swipe_gesture_recognizer`
// per [[vault/30_RFCs/RFC-0003-gesture-recognizers]].

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_button.hpp>
#include <mpapp/gestures/swipe_gesture_recognizer.hpp>
#include <mpapp/handlers/mock/view_handler.hpp>

using namespace mpapp;

TEST_CASE("swipe_gesture_recognizer attaches with MAUI defaults",
          "[mock][gesture][swipe]") {
    internal::basic_button b;
    auto& sw = b.add_gesture<swipe_gesture_recognizer>();

    REQUIRE(b.gesture_recognizers.size() == 1);
    CHECK(sw.kind() == internal::gesture_kind::swipe);
    CHECK(sw.direction.get() == swipe_direction::right);   // MAUI default
    CHECK(sw.threshold.get() == 100u);                     // MAUI default
}

TEST_CASE("mock view_handler records gesture.swipe_attached",
          "[mock][gesture][swipe]") {
    internal::basic_button b;
    b.add_gesture<swipe_gesture_recognizer>();

    view_handler<platform::mock> h;
    h.map_gestures(b);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "gesture.swipe_attached",
    });
}

TEST_CASE("simulate_swipe respects the recognizer's direction bitmask",
          "[mock][gesture][swipe]") {
    internal::basic_button b;
    auto& sw = b.add_gesture<swipe_gesture_recognizer>();
    sw.direction = swipe_direction::left | swipe_direction::right;

    int hits = 0;
    swiped_event_args last{};
    signal_slot<const swiped_event_args&> slot;
    struct cb_t {
        int* hits; swiped_event_args* last;
        void operator()(const swiped_event_args& a) const {
            ++*hits; *last = a;
        }
    } cb{&hits, &last};
    sw.swiped.subscribe(slot, cb);

    view_handler<platform::mock> h;
    h.simulate_swipe(b, swipe_direction::left);   // matches bitmask -> fires
    h.simulate_swipe(b, swipe_direction::right);  // matches -> fires
    h.simulate_swipe(b, swipe_direction::up);     // not in bitmask -> no fire
    h.simulate_swipe(b, swipe_direction::down);   // not in bitmask -> no fire

    CHECK(hits == 2);
    CHECK(last.direction == swipe_direction::right);
}

TEST_CASE("multiple swipe recognizers with disjoint directions fire independently",
          "[mock][gesture][swipe]") {
    internal::basic_button b;
    auto& horiz = b.add_gesture<swipe_gesture_recognizer>();
    auto& vert  = b.add_gesture<swipe_gesture_recognizer>();
    horiz.direction = swipe_direction::left | swipe_direction::right;
    vert.direction  = swipe_direction::up | swipe_direction::down;

    int horiz_hits = 0;
    int vert_hits  = 0;
    signal_slot<const swiped_event_args&> h_slot, v_slot;
    struct cb_t {
        int* counter;
        void operator()(const swiped_event_args&) const { ++*counter; }
    };
    cb_t hcb{&horiz_hits};
    cb_t vcb{&vert_hits};
    horiz.swiped.subscribe(h_slot, hcb);
    vert.swiped.subscribe(v_slot, vcb);

    view_handler<platform::mock> h;
    h.simulate_swipe(b, swipe_direction::left);
    h.simulate_swipe(b, swipe_direction::up);
    h.simulate_swipe(b, swipe_direction::right);

    CHECK(horiz_hits == 2);   // left + right
    CHECK(vert_hits  == 1);   // up only
}
