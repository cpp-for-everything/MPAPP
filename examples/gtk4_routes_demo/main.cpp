// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0017 — typed-routing demo (Linux/GTK4).
//
// Visible end-to-end exercise of:
//   * ADR-0016 — compile-time `route_table` + `shell::go_to<Path, &Table>(args...)`
//   * ADR-0023 — `can_activate` / `can_deactivate` guards + `navigation_blocked`
//   * ADR-0023 — `page::navigated_to` / `navigated_from` lifecycle
//
// UI: status labels (current_route, last navigation_blocked target,
// 6-line lifecycle log) + two switches (block_activate, form_dirty)
// + six buttons each wired to a compile-time-checked
// `shell::go_to<Path, &routes>(args...)`.

#include <string>
#include <vector>

#include <mpapp/application.hpp>
#include <mpapp/button.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/page.hpp>
#include <mpapp/route.hpp>
#include <mpapp/run.hpp>
#include <mpapp/shell.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/switch_.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/button_handler.hpp>
#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/switch_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

// Stand-in page subclasses — instances never instantiated here; route
// types carry through the table for shape compliance only.
struct home_page    : mpapp::page {};
struct details_page : mpapp::page {};
struct settings_page: mpapp::page {};
struct profile_page : mpapp::page {};
struct help_page    : mpapp::page {};
struct about_page   : mpapp::page {};

inline constexpr auto routes = mpapp::route_table{
    mpapp::route<"home",             home_page>{},
    mpapp::route<"home/details",     details_page, mpapp::param<"id", int>>{},
    mpapp::route<"settings",         settings_page>{},
    mpapp::route<"settings/profile", profile_page, mpapp::param<"who", std::string>>{},
    mpapp::route<"help",             help_page>{},
    mpapp::route<"help/about",       about_page,   mpapp::param<"section", int>>{},
};

class routes_demo_app : public mpapp::application {
public:
    void on_launch() override {
        // ----- Page lifecycle hookup --------------------------------
        current_page_.navigated_to.subscribe(nav_to_slot_, nav_to_cb_);
        current_page_.navigated_from.subscribe(nav_from_slot_, nav_from_cb_);
        shell_.current_content = &current_page_;

        // ----- Guards -----------------------------------------------
        shell_.can_activate = [this](std::string_view target) {
            if (block_activate_.is_on.get()) {
                last_blocked_target_ = std::string{target};
                return false;
            }
            return true;
        };
        shell_.can_deactivate = [this](std::string_view, std::string_view target) {
            if (form_dirty_.is_on.get()) {
                last_blocked_target_ = std::string{target};
                return false;
            }
            return true;
        };
        shell_.navigation_blocked.subscribe(nav_blocked_slot_, nav_blocked_cb_);
        shell_.current_route.changed.subscribe(route_slot_, route_cb_);

        // ----- Status labels ----------------------------------------
        bind_label(route_label_,          route_label_handler_,          "current_route: //");
        bind_label(blocked_label_,        blocked_label_handler_,        "navigation_blocked: (none)");
        bind_label(log_label_,            log_label_handler_,            "lifecycle: (no events yet)");
        bind_label(block_activate_label_, block_activate_label_handler_, "Block activate");
        bind_label(form_dirty_label_,     form_dirty_label_handler_,     "Form dirty (blocks deactivate)");

        // ----- Switches ---------------------------------------------
        block_activate_.set_handler(block_activate_handler_);
        form_dirty_.set_handler(form_dirty_handler_);
        block_activate_handler_.map_is_on(block_activate_);
        form_dirty_handler_.map_is_on(form_dirty_);
        block_activate_.is_on = false;
        form_dirty_.is_on     = false;

        // ----- Tabs (Shell wants its tab list even if we don't display them) ----
        shell_.add_tab("home");
        shell_.add_tab("settings");
        shell_.add_tab("help");

        // ----- Buttons — one member-function callback per button ----
        bind_button(btn_home_,             btn_home_handler_,
                    "go_to<\"home\", &routes>()",
                    btn_home_slot_,         btn_home_cb_);
        bind_button(btn_home_details_,     btn_home_details_handler_,
                    "go_to<\"home/details\", &routes>(42)",
                    btn_home_details_slot_, btn_home_details_cb_);
        bind_button(btn_settings_,         btn_settings_handler_,
                    "go_to<\"settings\", &routes>()",
                    btn_settings_slot_,     btn_settings_cb_);
        bind_button(btn_settings_profile_, btn_settings_profile_handler_,
                    "go_to<\"settings/profile\", &routes>(\"ada\")",
                    btn_settings_profile_slot_, btn_settings_profile_cb_);
        bind_button(btn_help_,             btn_help_handler_,
                    "go_to<\"help\", &routes>()",
                    btn_help_slot_,         btn_help_cb_);
        bind_button(btn_help_about_,       btn_help_about_handler_,
                    "go_to<\"help/about\", &routes>(4)",
                    btn_help_about_slot_,   btn_help_about_cb_);

        // ----- Compose ----------------------------------------------
        layout_.set_handler(layout_handler_);
        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing           = 8.0;
        layout_.padding           = mpapp::thickness{16.0};
        layout_.add(route_label_);
        layout_.add(blocked_label_);
        layout_.add(log_label_);
        layout_.add(block_activate_label_);
        layout_.add(block_activate_);
        layout_.add(form_dirty_label_);
        layout_.add(form_dirty_);
        layout_.add(btn_home_);
        layout_.add(btn_home_details_);
        layout_.add(btn_settings_);
        layout_.add(btn_settings_profile_);
        layout_.add(btn_help_);
        layout_.add(btn_help_about_);
        layout_handler_.bind(layout_);

        window_.title  = "MPAPP T-0017 - Typed Routing Demo (GTK4)";
        window_.width  = 580;
        window_.height = 720;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    using lp = mpapp::platform::current;

    void bind_label(mpapp::label& lbl,
                    mpapp::label_handler<lp>& h,
                    const std::string& text) {
        lbl.set_handler(h);
        h.map_text(lbl);
        lbl.text = text;
    }
    template <class Cb>
    void bind_button(mpapp::button& btn,
                     mpapp::button_handler<lp>& h,
                     const std::string& text,
                     mpapp::signal_slot<>& slot,
                     Cb& cb) {
        btn.set_handler(h);
        h.map_text(btn);
        h.map_clicked(btn);
        btn.text = text;
        btn.clicked.subscribe(slot, cb);
    }

    void append_log(const std::string& line) {
        log_history_.push_back(line);
        if (log_history_.size() > 6) log_history_.erase(log_history_.begin());
        std::string joined = "lifecycle:\n";
        for (const auto& l : log_history_) { joined += "  "; joined += l; joined += "\n"; }
        log_label_.text = joined;
    }

    // Per-button callback functors. Each has stable address (member).
    struct btn_home_cb_t              { routes_demo_app* self; void operator()() const { self->shell_.go_to<"home", &routes>(); } };
    struct btn_home_details_cb_t      { routes_demo_app* self; void operator()() const { self->shell_.go_to<"home/details", &routes>(42); } };
    struct btn_settings_cb_t          { routes_demo_app* self; void operator()() const { self->shell_.go_to<"settings", &routes>(); } };
    struct btn_settings_profile_cb_t  { routes_demo_app* self; void operator()() const { self->shell_.go_to<"settings/profile", &routes>(std::string{"ada"}); } };
    struct btn_help_cb_t              { routes_demo_app* self; void operator()() const { self->shell_.go_to<"help", &routes>(); } };
    struct btn_help_about_cb_t        { routes_demo_app* self; void operator()() const { self->shell_.go_to<"help/about", &routes>(4); } };

    struct route_cb_t        { routes_demo_app* self; void operator()(const std::string& v) const { self->route_label_.text = "current_route: " + v; } };
    struct nav_to_cb_t       { routes_demo_app* self; void operator()(const std::string& uri) const { self->append_log("navigated_to(" + uri + ")"); } };
    struct nav_from_cb_t     { routes_demo_app* self; void operator()(const std::string& prev) const { self->append_log("navigated_from(" + prev + ")"); } };
    struct nav_blocked_cb_t  { routes_demo_app* self; void operator()(const std::string& target) const { self->blocked_label_.text = "navigation_blocked: " + target; } };

    // ---- Members ------------------------------------------------------

    mpapp::shell shell_{};
    mpapp::page  current_page_{};

    mpapp::label route_label_{};
    mpapp::label blocked_label_{};
    mpapp::label log_label_{};
    mpapp::label block_activate_label_{};
    mpapp::label form_dirty_label_{};

    mpapp::switch_ block_activate_{};
    mpapp::switch_ form_dirty_{};

    mpapp::button btn_home_{};
    mpapp::button btn_home_details_{};
    mpapp::button btn_settings_{};
    mpapp::button btn_settings_profile_{};
    mpapp::button btn_help_{};
    mpapp::button btn_help_about_{};

    mpapp::stack_layout layout_{};
    mpapp::window       window_{};

    mpapp::label_handler<lp>        route_label_handler_{};
    mpapp::label_handler<lp>        blocked_label_handler_{};
    mpapp::label_handler<lp>        log_label_handler_{};
    mpapp::label_handler<lp>        block_activate_label_handler_{};
    mpapp::label_handler<lp>        form_dirty_label_handler_{};
    mpapp::switch_handler<lp>       block_activate_handler_{};
    mpapp::switch_handler<lp>       form_dirty_handler_{};
    mpapp::button_handler<lp>       btn_home_handler_{};
    mpapp::button_handler<lp>       btn_home_details_handler_{};
    mpapp::button_handler<lp>       btn_settings_handler_{};
    mpapp::button_handler<lp>       btn_settings_profile_handler_{};
    mpapp::button_handler<lp>       btn_help_handler_{};
    mpapp::button_handler<lp>       btn_help_about_handler_{};
    mpapp::stack_layout_handler<lp> layout_handler_{};
    mpapp::window_handler<lp>       window_handler_{};

    btn_home_cb_t              btn_home_cb_{this};
    btn_home_details_cb_t      btn_home_details_cb_{this};
    btn_settings_cb_t          btn_settings_cb_{this};
    btn_settings_profile_cb_t  btn_settings_profile_cb_{this};
    btn_help_cb_t              btn_help_cb_{this};
    btn_help_about_cb_t        btn_help_about_cb_{this};
    route_cb_t                 route_cb_{this};
    nav_to_cb_t                nav_to_cb_{this};
    nav_from_cb_t              nav_from_cb_{this};
    nav_blocked_cb_t           nav_blocked_cb_{this};

    mpapp::signal_slot<>                   btn_home_slot_{};
    mpapp::signal_slot<>                   btn_home_details_slot_{};
    mpapp::signal_slot<>                   btn_settings_slot_{};
    mpapp::signal_slot<>                   btn_settings_profile_slot_{};
    mpapp::signal_slot<>                   btn_help_slot_{};
    mpapp::signal_slot<>                   btn_help_about_slot_{};
    mpapp::signal_slot<const std::string&> route_slot_{};
    mpapp::signal_slot<const std::string&> nav_to_slot_{};
    mpapp::signal_slot<const std::string&> nav_from_slot_{};
    mpapp::signal_slot<const std::string&> nav_blocked_slot_{};

    std::vector<std::string> log_history_{};
    std::string              last_blocked_target_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<routes_demo_app>(argc, argv);
}
