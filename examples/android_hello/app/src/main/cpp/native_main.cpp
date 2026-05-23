// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android example native_main.
//
// Bridge between the Java MainActivity and `mpapp::run<spike_app>`.

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>

#include <jni.h>

#include <mpapp/collection_view.hpp>
#include <mpapp/detail/graphics/canvas.hpp>
#include <mpapp/hybrid_bridge.hpp>
#include <mpapp/label.hpp>
#include <mpapp/page.hpp>
#include <mpapp/route.hpp>
#include <mpapp/shell.hpp>

#include <android/log.h>

#if defined(MPAPP_GRAPHICS_HAS_CAIRO)
    #include <cairo/cairo.h>
#endif

#include <mpapp/application.hpp>
#include <mpapp/box_view.hpp>
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
#include <mpapp/switch_cell.hpp>
#include <mpapp/table_view.hpp>
#include <mpapp/text_cell.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/android/box_view_handler.hpp>
#include <mpapp/handlers/android/button_handler.hpp>
#include <mpapp/handlers/android/check_box_handler.hpp>
#include <mpapp/handlers/android/entry_handler.hpp>
#include <mpapp/handlers/android/jni_bridge.hpp>
#include <mpapp/handlers/android/label_handler.hpp>
#include <mpapp/handlers/android/scroll_view_handler.hpp>
#include <mpapp/handlers/android/slider_handler.hpp>
#include <mpapp/handlers/android/stack_layout_handler.hpp>
#include <mpapp/handlers/android/switch_cell_handler.hpp>
#include <mpapp/handlers/android/switch_handler.hpp>
#include <mpapp/handlers/android/table_view_handler.hpp>
#include <mpapp/handlers/android/text_cell_handler.hpp>
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
        box_.set_handler(box_handler_);
        // Cell-typed TableView demo — proves the typed_sections surface
        // composes on Android (mirrors gtk4_tableview_demo + windows_tableview_demo).
        profile_cell_.text   = "Profile";
        profile_cell_.detail = "Ada Lovelace";
        profile_cell_handler_.map_text(profile_cell_);
        profile_cell_handler_.map_detail(profile_cell_);
        profile_cell_.set_tc_handler(profile_cell_handler_);

        push_cell_.text = "Push notifications";
        push_cell_.on   = true;
        push_cell_handler_.map_text(push_cell_);
        push_cell_handler_.map_on(push_cell_);
        push_cell_.set_sc_handler(push_cell_handler_);

        tv_.typed_sections = std::vector<mpapp::table_section_typed>{
            mpapp::table_section_typed{
                "Account",
                std::vector<mpapp::cell*>{ &profile_cell_ }
            },
            mpapp::table_section_typed{
                "Preferences",
                std::vector<mpapp::cell*>{ &push_cell_ }
            },
        };
        tv_handler_.map_typed_sections(tv_);
        tv_handler_.map_sections(tv_);
        tv_handler_.map_row_height(tv_);
        tv_.set_tv_handler(tv_handler_);

        btn_.text         = "Click me";
        lbl_.text         = "Count: 0 — hello, world";
        name_.placeholder = "Type your name";
        shout_.is_on      = false;
        exclaim_.is_checked = false;
        repeat_.minimum   = 1.0;
        repeat_.maximum   = 5.0;
        repeat_.value     = 1.0;
        box_.fill         = mpapp::color{0.0, 0.6, 0.65, 1.0};   // teal
        box_.corners      = mpapp::corner_radius{4.0, 4.0, 4.0, 4.0};

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
        box_handler_.map_fill(box_);
        box_handler_.map_corners(box_);

        btn_.clicked.subscribe(click_slot_, click_cb_);
        vm_.count.changed.subscribe(count_slot_, count_cb_);
        name_.text.changed.subscribe(name_slot_, name_cb_);
        shout_.is_on.changed.subscribe(shout_slot_, shout_cb_);
        exclaim_.is_checked.changed.subscribe(exclaim_slot_, exclaim_cb_);
        repeat_.value.changed.subscribe(repeat_slot_, repeat_cb_);
        repeat_.value.changed.subscribe(box_corners_slot_, box_corners_cb_);

        layout_.stack_orientation    = mpapp::orientation::vertical;
        layout_.spacing              = 12.0;
        layout_.padding              = mpapp::thickness{24.0};
        layout_.horizontal_alignment = mpapp::h_align::center;
        layout_.vertical_alignment   = mpapp::v_align::center;
        layout_.add(box_);
        layout_.add(lbl_);
        layout_.add(name_);
        layout_.add(shout_);
        layout_.add(exclaim_);
        layout_.add(repeat_);
        layout_.add(btn_);
        layout_.add(tv_);   // typed_sections demo at the bottom of the stack
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
    struct box_corners_cb_t {
        spike_app* self;
        void operator()(double v) const {
            const double r = v * 4.0;  // slider 1.0 → 4px corners; 5.0 → 20px
            self->box_.corners.set(mpapp::corner_radius{r, r, r, r});
        }
    };

    view_model              vm_{};
    mpapp::button           btn_{};
    mpapp::label            lbl_{};
    mpapp::entry            name_{};
    mpapp::switch_          shout_{};
    mpapp::check_box        exclaim_{};
    mpapp::slider           repeat_{};
    mpapp::stack_layout     layout_{};
    mpapp::scroll_view      scroll_{};
    mpapp::box_view         box_{};
    mpapp::table_view       tv_{};
    mpapp::text_cell        profile_cell_{};
    mpapp::switch_cell      push_cell_{};
    mpapp::window           window_{};

    mpapp::button_handler<mpapp::platform::android>       btn_handler_{};
    mpapp::label_handler<mpapp::platform::android>        lbl_handler_{};
    mpapp::entry_handler<mpapp::platform::android>        name_handler_{};
    mpapp::switch_handler<mpapp::platform::android>       shout_handler_{};
    mpapp::check_box_handler<mpapp::platform::android>    exclaim_handler_{};
    mpapp::slider_handler<mpapp::platform::android>       repeat_handler_{};
    mpapp::stack_layout_handler<mpapp::platform::android> layout_handler_{};
    mpapp::scroll_view_handler<mpapp::platform::android>  scroll_handler_{};
    mpapp::box_view_handler<mpapp::platform::android>     box_handler_{};
    mpapp::table_view_handler<mpapp::platform::android>   tv_handler_{};
    mpapp::text_cell_handler<mpapp::platform::android>    profile_cell_handler_{};
    mpapp::switch_cell_handler<mpapp::platform::android>  push_cell_handler_{};
    mpapp::window_handler<mpapp::platform::android>       window_handler_{};

    click_cb_t                             click_cb_{this};
    count_cb_t                             count_cb_{this};
    name_cb_t                              name_cb_{this};
    shout_cb_t                             shout_cb_{this};
    exclaim_cb_t                           exclaim_cb_{this};
    repeat_cb_t                            repeat_cb_{this};
    box_corners_cb_t                       box_corners_cb_{this};
    mpapp::signal_slot<>                   click_slot_{};
    mpapp::signal_slot<const int&>         count_slot_{};
    mpapp::signal_slot<const std::string&> name_slot_{};
    mpapp::signal_slot<const bool&>        shout_slot_{};
    mpapp::signal_slot<const bool&>        exclaim_slot_{};
    mpapp::signal_slot<const double&>      repeat_slot_{};
    mpapp::signal_slot<const double&>      box_corners_slot_{};
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

// T-0017 — typed-routing smoke test. Exercises ADR-0016
// `route_table` + `shell::go_to<Path, &Table>(args...)` + ADR-0023
// `can_activate` / `can_deactivate` guards + page `navigated_to` /
// `navigated_from` lifecycle on the Android NDK toolchain. Output
// is a structured trace via logcat, prefixed `T-0017:` so test
// infra can grep it. No GUI dependency — proves the routing
// surface compiles + executes on Android the same as Win/Linux.
namespace t0017 {

struct home_page    : mpapp::page {};
struct details_page : mpapp::page {};
struct settings_page: mpapp::page {};

inline constexpr auto routes = mpapp::route_table{
    mpapp::route<"home",         home_page>{},
    mpapp::route<"home/details", details_page, mpapp::param<"id", int>>{},
    mpapp::route<"settings",     settings_page>{},
};

void log(const char* msg) {
    __android_log_print(ANDROID_LOG_INFO, "MPAPP", "T-0017: %s", msg);
}

void run_smoke() {
    mpapp::shell s;
    mpapp::page  current;
    s.current_content = &current;

    // Lifecycle counters
    int to_count = 0, from_count = 0, blocked_count = 0;
    bool block_activate   = false;
    bool block_deactivate = false;

    struct to_cb_t   { int* c; void operator()(const std::string&) const { ++*c; } };
    struct from_cb_t { int* c; void operator()(const std::string&) const { ++*c; } };
    struct blk_cb_t  { int* c; void operator()(const std::string&) const { ++*c; } };
    to_cb_t   to_cb{&to_count};
    from_cb_t from_cb{&from_count};
    blk_cb_t  blk_cb{&blocked_count};
    mpapp::signal_slot<const std::string&> to_slot{};
    mpapp::signal_slot<const std::string&> from_slot{};
    mpapp::signal_slot<const std::string&> blk_slot{};
    current.navigated_to.subscribe(to_slot, to_cb);
    current.navigated_from.subscribe(from_slot, from_cb);
    s.navigation_blocked.subscribe(blk_slot, blk_cb);

    s.can_activate = [&](std::string_view) { return !block_activate; };
    s.can_deactivate = [&](std::string_view, std::string_view) { return !block_deactivate; };

    // 1) Plain typed navigation, no guards.
    s.go_to<"home", &routes>();
    log(("after home: route=" + s.current_route.get()).c_str());

    s.go_to<"home/details", &routes>(42);
    log(("after details(42): route=" + s.current_route.get()).c_str());

    // 2) can_activate blocks.
    block_activate = true;
    s.go_to<"settings", &routes>();
    log(("after blocked settings: route=" + s.current_route.get()
         + " blocked_count=" + std::to_string(blocked_count)).c_str());
    block_activate = false;

    // 3) can_deactivate blocks.
    block_deactivate = true;
    s.go_to<"settings", &routes>();
    log(("after dirty-block: route=" + s.current_route.get()
         + " blocked_count=" + std::to_string(blocked_count)).c_str());
    block_deactivate = false;

    // 4) Successful navigation after unblocking.
    s.go_to<"settings", &routes>();
    log(("after settings: route=" + s.current_route.get()).c_str());

    // 5) Lifecycle totals — to=4 (home, details, settings, settings)
    //    from=3 (home, details, settings since the first go_to
    //    skips because current_route is "//" — no previous page event)
    log(("totals: to=" + std::to_string(to_count)
         + " from=" + std::to_string(from_count)
         + " blocked=" + std::to_string(blocked_count)).c_str());
}

} // namespace t0017

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_example_MainActivity_nativeRunRoutesSmokeTest(
    JNIEnv* /*env*/, jobject /*thiz*/) {
    t0017::run_smoke();
}

// T-0018 — async bridge dispatch smoke test. Exercises ADR-0018
// Phase F: `hybrid_bridge::dispatch_async` over a sync method
// (`add_sync`), an inline-responding async method
// (`add_async_inline`), and a deferred-respond async method
// (`defer_add` — the bridge captures `respond`, the smoke fires it
// later). Output is a structured trace via logcat, prefixed
// `T-0018:` so test infra can grep it.
namespace t0018 {

class demo_bridge : public mpapp::hybrid_bridge {
public:
    demo_bridge() {
        register_method("add_sync",        &demo_bridge::add_sync);
        register_async_method<int>("add_async_inline",
                                   &demo_bridge::add_async_inline);
        register_async_method<int>("defer_add",
                                   &demo_bridge::defer_add);
    }

    int add_sync(int a, int b) { return a + b; }

    void add_async_inline(int a, int b, std::function<void(int)> respond) {
        respond(a + b);
    }

    void defer_add(int a, int b, std::function<void(int)> respond) {
        pending_a_       = a;
        pending_b_       = b;
        pending_respond_ = std::move(respond);
    }

    void resolve_pending() {
        if (!pending_respond_) return;
        auto cb = std::move(pending_respond_);
        pending_respond_ = nullptr;
        cb(pending_a_ + pending_b_);
    }

    bool has_pending() const { return static_cast<bool>(pending_respond_); }

private:
    int                      pending_a_ = 0;
    int                      pending_b_ = 0;
    std::function<void(int)> pending_respond_;
};

void log(const char* msg) {
    __android_log_print(ANDROID_LOG_INFO, "MPAPP", "T-0018: %s", msg);
}

void run_smoke() {
    demo_bridge b;

    // 1) Sync dispatch — fires inline. Result in `out`.
    {
        std::string out;
        b.dispatch(R"({"id":1,"method":"add_sync","args":[2,3]})", out);
        log(("sync add_sync(2,3) -> " + out).c_str());
    }

    // 2) Async-inline dispatch — fires inline through dispatch_async.
    {
        std::string captured;
        bool        fired = false;
        b.dispatch_async(R"({"id":2,"method":"add_async_inline","args":[10,20]})",
                         [&](std::string r) { captured = std::move(r); fired = true; });
        log((std::string{"inline add_async_inline(10,20) fired_before_return="}
             + (fired ? "true" : "false")
             + " -> " + captured).c_str());
    }

    // 3) Async-deferred dispatch — bridge captures `respond`, smoke
    //    fires it later via resolve_pending.
    {
        std::string captured;
        bool        fired = false;
        b.dispatch_async(R"({"id":3,"method":"defer_add","args":[7,8]})",
                         [&](std::string r) { captured = std::move(r); fired = true; });
        log((std::string{"deferred defer_add(7,8) fired_before_resolve="}
             + (fired ? "true" : "false")
             + " has_pending="
             + (b.has_pending() ? "true" : "false")).c_str());
        b.resolve_pending();
        log((std::string{"deferred defer_add(7,8) fired_after_resolve="}
             + (fired ? "true" : "false")
             + " -> " + captured).c_str());
    }

    // 4) Unknown-method dispatch — error envelope.
    {
        std::string captured;
        b.dispatch_async(R"({"id":4,"method":"missing","args":[]})",
                         [&](std::string r) { captured = std::move(r); });
        log(("unknown-method -> " + captured).c_str());
    }
}

} // namespace t0018

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_example_MainActivity_nativeRunBridgeSmokeTest(
    JNIEnv* /*env*/, jobject /*thiz*/) {
    t0018::run_smoke();
}

// T-0019 — CollectionView item_template smoke test. Exercises the
// factory-based typed-cells surface: registers an item_template,
// rotates items_source between three vectors, observes
// materialized_count + materialized_changed + factory call indices.
// Output is structured logcat, prefixed `T-0019:`.
namespace t0019 {

void log(const char* msg) {
    __android_log_print(ANDROID_LOG_INFO, "MPAPP", "T-0019: %s", msg);
}

void run_smoke() {
    mpapp::collection_view cv;

    int factory_invocations  = 0;
    int last_factory_index   = -1;
    int materialized_changes = 0;

    struct mat_cb_t { int* c; void operator()() const { ++*c; } };
    mat_cb_t mat_cb{&materialized_changes};
    mpapp::signal_slot<> mat_slot{};
    cv.materialized_changed.subscribe(mat_slot, mat_cb);

    // Factory: emits a fresh mpapp::label per row. No handler attached
    // here — the smoke is concerned with the model-level materialize
    // surface, not the native render path. The label is owned by the
    // collection_view via the unique_ptr returned from the factory.
    cv.item_template = [&](int i) -> std::unique_ptr<mpapp::view> {
        ++factory_invocations;
        last_factory_index = i;
        auto l = std::make_unique<mpapp::label>();
        l->text = "row " + std::to_string(i);
        return l;
    };

    // 1) First items_source — 4 rows.
    cv.items_source = std::vector<std::string>{"a", "b", "c", "d"};
    log(("after items=4: mat_count=" + std::to_string(cv.materialized_count())
         + " factory_invocations=" + std::to_string(factory_invocations)
         + " last_index=" + std::to_string(last_factory_index)
         + " mat_changes=" + std::to_string(materialized_changes)).c_str());

    // 2) Rotate to 3 rows. Old materialized cells are dropped, factory
    //    is called 3 more times.
    cv.items_source = std::vector<std::string>{"x", "y", "z"};
    log(("after items=3: mat_count=" + std::to_string(cv.materialized_count())
         + " factory_invocations=" + std::to_string(factory_invocations)
         + " last_index=" + std::to_string(last_factory_index)
         + " mat_changes=" + std::to_string(materialized_changes)).c_str());

    // 3) Rotate to 5 rows.
    cv.items_source = std::vector<std::string>{"p", "q", "r", "s", "t"};
    log(("after items=5: mat_count=" + std::to_string(cv.materialized_count())
         + " factory_invocations=" + std::to_string(factory_invocations)
         + " last_index=" + std::to_string(last_factory_index)
         + " mat_changes=" + std::to_string(materialized_changes)).c_str());

    // 4) Clear template — materialized_count drops to 0, factory not
    //    called again.
    cv.item_template = mpapp::collection_view::item_factory_t{};
    log(("after clear-template: mat_count=" + std::to_string(cv.materialized_count())
         + " factory_invocations=" + std::to_string(factory_invocations)
         + " mat_changes=" + std::to_string(materialized_changes)).c_str());
}

} // namespace t0019

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_example_MainActivity_nativeRunItemTemplateSmokeTest(
    JNIEnv* /*env*/, jobject /*thiz*/) {
    t0019::run_smoke();
}

// T-0016 — Cairo render demo. Drives the canvas facade through a
// representative paint sequence and writes the result as PNG to the
// path supplied by MainActivity. Returns 0 on success, 1 on cairo
// failure, 2 on backend missing.
//
// The paint sequence is intentionally identical to the cross-platform
// `examples/cairo_render_demo/cairo_render_demo.cpp` CLI so the three
// per-platform PNGs are pixel-identical when the backend is real.
extern "C" JNIEXPORT jint JNICALL
Java_io_mpapp_example_MainActivity_nativeRenderCairoDemoPng(
    JNIEnv* env, jobject /*thiz*/, jstring jpath) {
#if defined(MPAPP_GRAPHICS_HAS_CAIRO)
    if (jpath == nullptr) return 1;
    const char* path = env->GetStringUTFChars(jpath, nullptr);
    if (path == nullptr) return 1;

    // Build an independent cairo surface for PNG writeback (the
    // facade doesn't expose its surface through the abstract API).
    cairo_surface_t* surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 320);
    cairo_t* cr = cairo_create(surface);

    // Replicate the cross-platform demo's paint sequence directly
    // through Cairo. Kept inline so we don't need to ship a separate
    // header for the shared scene.
    cairo_set_source_rgba(cr, 0.96, 0.96, 0.94, 1.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    auto fill_rect = [&](double r, double g, double b, double x, double y, double w, double h) {
        cairo_set_source_rgba(cr, r, g, b, 1.0);
        cairo_rectangle(cr, x, y, w, h);
        cairo_fill(cr);
    };
    auto fill_ellipse = [&](double r, double g, double b, double x, double y, double w, double h) {
        cairo_set_source_rgba(cr, r, g, b, 1.0);
        cairo_save(cr);
        cairo_translate(cr, x + w / 2.0, y + h / 2.0);
        cairo_scale(cr, w / 2.0, h / 2.0);
        cairo_arc(cr, 0, 0, 1, 0, 2 * 3.14159265358979323846);
        cairo_restore(cr);
        cairo_fill(cr);
    };

    fill_rect   (0.902, 0.224, 0.275,  20,  20, 100, 60);  // #E63946
    fill_ellipse(0.165, 0.616, 0.561, 140,  20, 100, 60);  // #2A9D8F
    cairo_set_source_rgba(cr, 0.957, 0.635, 0.380, 1.0);   // #F4A261
    cairo_move_to(cr, 260, 20); cairo_line_to(cr, 360, 20);
    cairo_line_to(cr, 310, 80); cairo_close_path(cr);
    cairo_fill(cr);

    cairo_set_source_rgba(cr, 0.114, 0.208, 0.341, 1.0);   // #1D3557
    cairo_set_line_width(cr, 3.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_rectangle(cr, 20, 110, 100, 60); cairo_stroke(cr);
    cairo_save(cr);
    cairo_translate(cr, 190, 140); cairo_scale(cr, 50, 30);
    cairo_arc(cr, 0, 0, 1, 0, 2 * 3.14159265358979323846);
    cairo_restore(cr); cairo_stroke(cr);
    cairo_move_to(cr, 260, 170);
    cairo_curve_to(cr, 290, 110, 330, 110, 360, 170);
    cairo_stroke(cr);

    cairo_save(cr);
    cairo_translate(cr, 80, 240);
    cairo_rotate(cr, 0.3);
    cairo_set_source_rgba(cr, 0.149, 0.275, 0.325, 0.5);  // #264653 50%
    cairo_rectangle(cr, 0, 0, 80, 50); cairo_fill(cr);
    cairo_restore(cr);

    cairo_save(cr);
    cairo_move_to(cr, 220, 245);
    cairo_curve_to(cr, 280, 215, 360, 215, 360, 275);
    cairo_curve_to(cr, 360, 305, 280, 305, 220, 275);
    cairo_close_path(cr); cairo_clip(cr);
    cairo_set_source_rgba(cr, 0.906, 0.435, 0.318, 1.0);  // #E76F51
    cairo_rectangle(cr, 200, 220, 200, 80); cairo_fill(cr);
    cairo_restore(cr);

    cairo_surface_flush(surface);
    auto status = cairo_surface_write_to_png(surface, path);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    env->ReleaseStringUTFChars(jpath, path);
    return status == CAIRO_STATUS_SUCCESS ? 0 : 1;
#else
    (void)env; (void)jpath;
    return 2;
#endif
}
