// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::pointer_gesture_recognizer`
// per [[vault/30_RFCs/RFC-0003-gesture-recognizers]].

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/button.hpp>
#include <mpapp/gestures/pointer_gesture_recognizer.hpp>
#include <mpapp/handlers/mock/view_handler.hpp>

using namespace mpapp;

namespace {

struct pointer_log_cb {
    std::vector<std::string>* trail;
    std::string                tag;
    void operator()(const pointer_event_args&) const { trail->push_back(tag); }
};

} // namespace

TEST_CASE("pointer_gesture_recognizer attaches with MAUI defaults",
          "[mock][gesture][pointer]") {
    internal::basic_button b;
    auto& p = b.add_gesture<pointer_gesture_recognizer>();

    REQUIRE(b.gesture_recognizers.size() == 1);
    CHECK(p.kind() == internal::gesture_kind::pointer);
    CHECK(p.buttons.get() == button_mask::primary);
}

TEST_CASE("mock view_handler records gesture.pointer_attached",
          "[mock][gesture][pointer]") {
    internal::basic_button b;
    b.add_gesture<pointer_gesture_recognizer>();

    view_handler<platform::mock> h;
    h.map_gestures(b);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "gesture.pointer_attached",
    });
}

TEST_CASE("simulate_pointer_* drives the five signals in order",
          "[mock][gesture][pointer]") {
    internal::basic_button b;
    auto& p = b.add_gesture<pointer_gesture_recognizer>();

    std::vector<std::string> trail;
    signal_slot<const pointer_event_args&> s_en, s_mv, s_pr, s_rl, s_ex;
    pointer_log_cb cb_en{&trail, "entered"};
    pointer_log_cb cb_mv{&trail, "moved"};
    pointer_log_cb cb_pr{&trail, "pressed"};
    pointer_log_cb cb_rl{&trail, "released"};
    pointer_log_cb cb_ex{&trail, "exited"};
    p.pointer_entered .subscribe(s_en, cb_en);
    p.pointer_moved   .subscribe(s_mv, cb_mv);
    p.pointer_pressed .subscribe(s_pr, cb_pr);
    p.pointer_released.subscribe(s_rl, cb_rl);
    p.pointer_exited  .subscribe(s_ex, cb_ex);

    view_handler<platform::mock> h;
    h.simulate_pointer_entered (b,  10.0, 10.0);
    h.simulate_pointer_moved   (b,  15.0, 12.0);
    h.simulate_pointer_pressed (b,  15.0, 12.0, button_mask::primary);
    h.simulate_pointer_released(b,  15.0, 12.0, button_mask::primary);
    h.simulate_pointer_exited  (b,  16.0, 13.0);

    REQUIRE(trail == std::vector<std::string>{
        "entered", "moved", "pressed", "released", "exited",
    });
}

TEST_CASE("pointer simulations fan out to multiple recognizers on one view",
          "[mock][gesture][pointer]") {
    internal::basic_button b;
    auto& a = b.add_gesture<pointer_gesture_recognizer>();
    auto& c = b.add_gesture<pointer_gesture_recognizer>();

    int a_hits = 0;
    int c_hits = 0;
    signal_slot<const pointer_event_args&> a_slot, c_slot;
    struct count_cb {
        int* hits;
        void operator()(const pointer_event_args&) const { ++*hits; }
    };
    count_cb a_cb{&a_hits};
    count_cb c_cb{&c_hits};
    a.pointer_moved.subscribe(a_slot, a_cb);
    c.pointer_moved.subscribe(c_slot, c_cb);

    view_handler<platform::mock> h;
    h.simulate_pointer_moved(b, 1, 2);
    h.simulate_pointer_moved(b, 3, 4);

    CHECK(a_hits == 2);
    CHECK(c_hits == 2);
}
