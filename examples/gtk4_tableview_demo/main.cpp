// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. TableView typed_sections demo — exercises the cell
// tree (text_cell / entry_cell / switch_cell) inside a TableView via
// the typed_sections Observable on Linux (GTK4 + GtkListBox).
//
// What this shows:
//
//   * `typed_sections` is the parallel surface to `sections` — it
//     carries `vector<table_section_typed{title, vec<cell*>}>`. The
//     handler renders each cell via its native widget through
//     ADR-0013 dispatch.
//   * Two-way bindings light up end-to-end: typing into the email
//     entry_cell sets `email_cell.text`, and flipping the
//     notifications switch_cell emits `on_changed`. Both update a
//     status `label` so the UI demonstrates the bidirectional flow.
//   * `completed` signal on entry_cell fires when the user presses
//     Enter inside the inline editor.
//
// Goal: zero direct gtk_* tokens in user-facing code; the platform
// bootstrap lives inside the GTK4 handler set and is invoked by
// `mpapp::run<App>`.

#include <string>

#include <mpapp/application.hpp>
#include <mpapp/entry_cell.hpp>
#include <mpapp/label.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/run.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/stack_layout.hpp>
#include <mpapp/switch_cell.hpp>
#include <mpapp/table_view.hpp>
#include <mpapp/text_cell.hpp>
#include <mpapp/window.hpp>

#include <mpapp/handlers/entry_cell_handler.hpp>
#include <mpapp/handlers/label_handler.hpp>
#include <mpapp/handlers/stack_layout_handler.hpp>
#include <mpapp/handlers/switch_cell_handler.hpp>
#include <mpapp/handlers/table_view_handler.hpp>
#include <mpapp/handlers/text_cell_handler.hpp>
#include <mpapp/handlers/window_handler.hpp>

namespace {

class tableview_demo_app : public mpapp::application {
public:
    void on_launch() override {
        // ---- Cells ----------------------------------------------------
        name_cell_.text   = "Name";
        name_cell_.detail = "Ada Lovelace";
        name_cell_handler_.map_text(name_cell_);
        name_cell_handler_.map_detail(name_cell_);
        name_cell_.set_tc_handler(name_cell_handler_);

        email_cell_.label       = "Email";
        email_cell_.text        = "ada@analytical.engine";
        email_cell_.placeholder = "name@example.com";
        email_cell_handler_.map_label(email_cell_);
        email_cell_handler_.map_text(email_cell_);
        email_cell_handler_.map_placeholder(email_cell_);
        email_cell_handler_.map_keyboard(email_cell_);
        email_cell_.set_ec_handler(email_cell_handler_);

        notifications_cell_.text = "Push notifications";
        notifications_cell_.on   = true;
        notifications_cell_handler_.map_text(notifications_cell_);
        notifications_cell_handler_.map_on(notifications_cell_);
        notifications_cell_.set_sc_handler(notifications_cell_handler_);

        // ---- TableView ------------------------------------------------
        tv_.typed_sections = std::vector<mpapp::table_section_typed>{
            mpapp::table_section_typed{
                "Profile",
                std::vector<mpapp::cell*>{ &name_cell_, &email_cell_ }
            },
            mpapp::table_section_typed{
                "Preferences",
                std::vector<mpapp::cell*>{ &notifications_cell_ }
            },
        };
        tv_handler_.map_typed_sections(tv_);
        tv_handler_.map_sections(tv_);   // still call so flat fallback is bound
        tv_handler_.map_row_height(tv_);
        tv_.set_tv_handler(tv_handler_);

        // ---- Status label (proves bindings round-trip) ----------------
        status_.text = "Bindings live — try editing or toggling rows";
        status_handler_.map_text(status_);
        status_.set_handler(status_handler_);

        // Bindings: email-text + switch-on both push into status_.
        email_cell_.text.changed.subscribe(email_slot_, email_cb_);
        notifications_cell_.on.changed.subscribe(notif_slot_, notif_cb_);
        email_cell_.completed.subscribe(completed_slot_, completed_cb_);

        // ---- Layout ---------------------------------------------------
        layout_.stack_orientation = mpapp::orientation::vertical;
        layout_.spacing           = 8.0;
        layout_.padding           = mpapp::thickness{16.0};
        layout_.add(tv_);
        layout_.add(status_);
        layout_handler_.bind(layout_);

        // ---- Window ---------------------------------------------------
        window_.title  = "MPAPP TableView typed_sections demo";
        window_.width  = 480;
        window_.height = 480;
        window_.set_handler(window_handler_);
        window_handler_.bind(window_);
        window_.content = &layout_;
        window_.show();
    }

private:
    void render_status(const std::string& last_event) {
        const std::string email = email_cell_.text.get();
        const std::string notif = notifications_cell_.on.get() ? "on" : "off";
        status_.text.set(last_event
            + "  ·  email=" + email
            + "  ·  notifications=" + notif);
    }

    struct email_cb_t {
        tableview_demo_app* self;
        void operator()(const std::string&) const { self->render_status("text changed"); }
    };
    struct notif_cb_t {
        tableview_demo_app* self;
        void operator()(bool) const { self->render_status("switch flipped"); }
    };
    struct completed_cb_t {
        tableview_demo_app* self;
        void operator()(const std::string&) const { self->render_status("entry completed"); }
    };

    // Surface
    mpapp::table_view    tv_{};
    mpapp::text_cell     name_cell_{};
    mpapp::entry_cell    email_cell_{};
    mpapp::switch_cell   notifications_cell_{};
    mpapp::label         status_{};
    mpapp::stack_layout  layout_{};
    mpapp::window        window_{};

    // Linux handlers
    mpapp::table_view_handler<>    tv_handler_{};
    mpapp::text_cell_handler<>     name_cell_handler_{};
    mpapp::entry_cell_handler<>    email_cell_handler_{};
    mpapp::switch_cell_handler<>   notifications_cell_handler_{};
    mpapp::label_handler<>         status_handler_{};
    mpapp::stack_layout_handler<>  layout_handler_{};
    mpapp::window_handler<>        window_handler_{};

    // Callbacks
    email_cb_t                             email_cb_{this};
    notif_cb_t                             notif_cb_{this};
    completed_cb_t                         completed_cb_{this};
    mpapp::signal_slot<const std::string&> email_slot_{};
    mpapp::signal_slot<const bool&>        notif_slot_{};
    mpapp::signal_slot<const std::string&> completed_slot_{};
};

} // namespace

int main(int argc, char** argv) {
    return mpapp::run<tableview_demo_app>(argc, argv);
}
