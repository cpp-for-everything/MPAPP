// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android example native_main.
//
// Bridge between the Java MainActivity and `mpapp::run<spike_app>`.

#include <algorithm>
#include <cctype>
#include <string>

#include <jni.h>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/check_box.hpp>
#include <mpapp/entry.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/scroll_view.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/slider.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/switch_.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/android/button_handler.hpp>
#include <mpapp/handlers/android/check_box_handler.hpp>
#include <mpapp/handlers/android/entry_handler.hpp>
#include <mpapp/handlers/android/jni_bridge.hpp>
#include <mpapp/handlers/android/label_handler.hpp>
#include <mpapp/handlers/android/scroll_view_handler.hpp>
#include <mpapp/handlers/android/slider_handler.hpp>
#include <mpapp/handlers/android/stack_layout_handler.hpp>
#include <mpapp/handlers/android/switch_handler.hpp>
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
        name_.set_handler(name_handler_);
        shout_.set_handler(shout_handler_);
        exclaim_.set_handler(exclaim_handler_);
        repeat_.set_handler(repeat_handler_);
        layout_.set_handler(layout_handler_);
        scroll_.set_handler(scroll_handler_);

        btn_.text         = "Click me";
        lbl_.text         = "Count: 0 — hello, world";
        name_.placeholder = "Type your name";
        shout_.is_on      = false;
        exclaim_.is_checked = false;
        repeat_.minimum   = 1.0;
        repeat_.maximum   = 5.0;
        repeat_.value     = 1.0;

        btn_handler_.map_text(btn_);
        btn_handler_.map_clicked(btn_);
        lbl_handler_.map_text(lbl_);
        name_handler_.map_text(name_);
        name_handler_.map_placeholder(name_);
        shout_handler_.map_is_on(shout_);
        exclaim_handler_.map_is_checked(exclaim_);
        repeat_handler_.map_minimum(repeat_);
        repeat_handler_.map_maximum(repeat_);
        repeat_handler_.map_value(repeat_);
        scroll_handler_.map_content(scroll_);
        scroll_handler_.map_orientation(scroll_);

        btn_.clicked.subscribe(click_slot_, click_cb_);
        vm_.count.changed.subscribe(count_slot_, count_cb_);
        name_.text.changed.subscribe(name_slot_, name_cb_);
        shout_.is_on.changed.subscribe(shout_slot_, shout_cb_);
        exclaim_.is_checked.changed.subscribe(exclaim_slot_, exclaim_cb_);
        repeat_.value.changed.subscribe(repeat_slot_, repeat_cb_);

        layout_.stack_orientation    = mpapp::orientation::vertical;
        layout_.spacing              = 12.0;
        layout_.padding              = mpapp::thickness{24.0};
        layout_.horizontal_alignment = mpapp::h_align::center;
        layout_.vertical_alignment   = mpapp::v_align::center;
        layout_.add(lbl_);
        layout_.add(name_);
        layout_.add(shout_);
        layout_.add(exclaim_);
        layout_.add(repeat_);
        layout_.add(btn_);
        layout_handler_.bind(layout_);
        scroll_handler_.bind_content(scroll_, layout_);

        window_.title  = "MPAPP T-0011 - Android hello";
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &scroll_;
        window_.show();
    }

private:
    static std::string greeting(const std::string& name, bool shout, bool exclaim) {
        const std::string who = name.empty() ? std::string{"world"} : name;
        std::string g = "hello, " + who;
        if (shout) {
            for (auto& c : g) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        if (exclaim) g += "!!!";
        return g;
    }

    void render_label() {
        const int n     = vm_.count.get();
        const int times = std::max(1, static_cast<int>(repeat_.value.get() + 0.5));
        std::string greet = greeting(name_.text.get(),
                                      shout_.is_on.get(),
                                      exclaim_.is_checked.get());
        std::string repeated;
        for (int i = 0; i < times; ++i) {
            if (i > 0) repeated += " · ";
            repeated += greet;
        }
        lbl_.text.set("Count: " + std::to_string(n) + " — " + repeated);
    }

    struct click_cb_t {
        spike_app* self;
        void operator()() const { self->vm_.count.set(self->vm_.count.get() + 1); }
    };
    struct count_cb_t  { spike_app* self; void operator()(int) const { self->render_label(); } };
    struct name_cb_t   { spike_app* self; void operator()(const std::string&) const { self->render_label(); } };
    struct shout_cb_t  { spike_app* self; void operator()(bool) const { self->render_label(); } };
    struct exclaim_cb_t{ spike_app* self; void operator()(bool) const { self->render_label(); } };
    struct repeat_cb_t { spike_app* self; void operator()(double) const { self->render_label(); } };

    view_model              vm_{};
    mpapp::button           btn_{};
    mpapp::label            lbl_{};
    mpapp::entry            name_{};
    mpapp::switch_          shout_{};
    mpapp::check_box        exclaim_{};
    mpapp::slider           repeat_{};
    mpapp::stack_layout     layout_{};
    mpapp::scroll_view      scroll_{};
    mpapp::window           window_{};

    mpapp::button_handler<mpapp::platform::android>       btn_handler_{};
    mpapp::label_handler<mpapp::platform::android>        lbl_handler_{};
    mpapp::entry_handler<mpapp::platform::android>        name_handler_{};
    mpapp::switch_handler<mpapp::platform::android>       shout_handler_{};
    mpapp::check_box_handler<mpapp::platform::android>    exclaim_handler_{};
    mpapp::slider_handler<mpapp::platform::android>       repeat_handler_{};
    mpapp::stack_layout_handler<mpapp::platform::android> layout_handler_{};
    mpapp::scroll_view_handler<mpapp::platform::android>  scroll_handler_{};
    mpapp::window_handler<mpapp::platform::android>       window_handler_{};

    click_cb_t                             click_cb_{this};
    count_cb_t                             count_cb_{this};
    name_cb_t                              name_cb_{this};
    shout_cb_t                             shout_cb_{this};
    exclaim_cb_t                           exclaim_cb_{this};
    repeat_cb_t                            repeat_cb_{this};
    mpapp::signal_slot<>                   click_slot_{};
    mpapp::signal_slot<const int&>         count_slot_{};
    mpapp::signal_slot<const std::string&> name_slot_{};
    mpapp::signal_slot<const bool&>        shout_slot_{};
    mpapp::signal_slot<const bool&>        exclaim_slot_{};
    mpapp::signal_slot<const double&>      repeat_slot_{};
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
