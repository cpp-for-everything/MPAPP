// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android example native_main.
//
// Bridge between the Java MainActivity and `mpapp::run<spike_app>`.

#include <string>

#include <jni.h>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/android/button_handler.hpp>
#include <mpapp/handlers/android/jni_bridge.hpp>
#include <mpapp/handlers/android/label_handler.hpp>
#include <mpapp/handlers/android/stack_layout_handler.hpp>
#include <mpapp/handlers/android/window_handler.hpp>

namespace {

struct view_model {
    mpapp::Observable<int> count{0};
};

class spike_app : public mpapp::application {
public:
    void on_launch() override {
        btn_.set_handler(btn_handler_);
        lbl_.set_handler(lbl_handler_);
        layout_.set_handler(layout_handler_);

        btn_.text = "Click me";
        lbl_.text = "Count: 0";

        btn_handler_.map_text(btn_);
        btn_handler_.map_clicked(btn_);
        lbl_handler_.map_text(lbl_);

        btn_.clicked.subscribe(click_slot_, click_cb_);
        vm_.count.changed.subscribe(count_slot_, count_cb_);

        layout_.stack_orientation    = mpapp::orientation::vertical;
        layout_.spacing              = 12.0;
        layout_.padding              = mpapp::thickness{24.0};
        layout_.horizontal_alignment = mpapp::h_align::center;
        layout_.vertical_alignment   = mpapp::v_align::center;
        layout_.add(lbl_);
        layout_.add(btn_);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0011 - Android hello";
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    struct click_cb_t {
        spike_app* self;
        void operator()() const { self->vm_.count.set(self->vm_.count.get() + 1); }
    };
    struct count_cb_t {
        spike_app* self;
        void operator()(int n) const { self->lbl_.text.set("Count: " + std::to_string(n)); }
    };

    view_model              vm_{};
    mpapp::button           btn_{};
    mpapp::label            lbl_{};
    mpapp::stack_layout     layout_{};
    mpapp::window           window_{};

    mpapp::button_handler<mpapp::platform::android>       btn_handler_{};
    mpapp::label_handler<mpapp::platform::android>        lbl_handler_{};
    mpapp::stack_layout_handler<mpapp::platform::android> layout_handler_{};
    mpapp::window_handler<mpapp::platform::android>       window_handler_{};

    click_cb_t                       click_cb_{this};
    count_cb_t                       count_cb_{this};
    mpapp::signal_slot<>             click_slot_{};
    mpapp::signal_slot<const int&>   count_slot_{};
};

} // namespace

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_example_MainActivity_nativeRegisterActivity(
    JNIEnv* env, jobject /*thiz*/, jobject activity) {
    mpapp::detail::set_activity(env, activity);
}

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_example_MainActivity_nativeLaunch(JNIEnv* /*env*/, jobject /*thiz*/) {
    char* argv[] = {const_cast<char*>("mpapp")};
    mpapp::run<spike_app>(1, argv);
}
