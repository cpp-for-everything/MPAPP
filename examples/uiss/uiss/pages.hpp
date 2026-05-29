// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. УИСС demo — page composition.
//
// One reusable `section_page` (header bar with a ≡ flyout toggle + title,
// over a scrollable content column) plus the `login_page` and the ten
// per-section content builders. Every builder binds to a single
// `uiss::student` record — no platform code, no ifdefs.

#ifndef UISS_PAGES_HPP
#define UISS_PAGES_HPP

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

#include <mpapp/carousel_view.hpp>
#include <mpapp/entry.hpp>
#include <mpapp/gestures/tap_gesture_recognizer.hpp>
#include <mpapp/image.hpp>
#include <mpapp/label.hpp>
#include <mpapp/page.hpp>
#include <mpapp/picker.hpp>
#include <mpapp/scroll_view.hpp>
#include <mpapp/signal.hpp>

#include "data.hpp"
#include "support.hpp"

namespace uiss {

// ---- Reusable scrollable section page ----------------------------------
//
// Layout tree:
//   page → root(vbox)
//            ├── header(hbox): [≡] title
//            └── scroll → body(vbox): data rows…
struct section_page {
    mpapp::page         page{};
    box                 root{};
    box                 header{};
    click_button        menu_btn{};
    mpapp::label        title_lbl{};
    mpapp::scroll_view  scroll{};
    box                 body{};
    label_list          labels{};

    // Optional picker (only Плащания uses one) — heap-allocated so the
    // other nine sections pay nothing for it. Its RFC-0014 command updates
    // picker_sel_lbl on selection (demonstrates control-side command on a
    // non-button control).
    std::unique_ptr<mpapp::picker> opt_picker{};
    mpapp::label*                  picker_sel_lbl = nullptr;

    // Optional announcements carousel (only Информация uses one) — a real
    // CarouselView (GtkStack / FlipView / ViewFlipper) paged by ◀ ▶ buttons
    // that drive carousel.scroll_to → the native page switch.
    std::unique_ptr<mpapp::carousel_view> opt_carousel{};
    std::unique_ptr<click_button>         car_prev{};
    std::unique_ptr<click_button>         car_next{};
    box                                   car_nav{};

    // Begin a page: header bar + an empty content column ready for rows.
    void begin(const std::string& title, std::function<void()> on_menu) {
        title_lbl.text       = title;
        title_lbl.font_bold  = true;
        title_lbl.font_size  = 20.0;
        title_lbl.text_color = tu_blue;
        menu_btn.build("≡", std::move(on_menu));   // ≡
        // Accessibility: the glyph alone is meaningless to a screen reader.
        menu_btn.btn.semantic_description = "Отвори навигацията";
        header.horizontal(gap_md, mpapp::thickness{pad_md})
              .add(menu_btn.btn)
              .add(title_lbl);
        body.vertical(gap_sm, mpapp::thickness{pad_lg});
    }

    // A bold-ish section heading (plain text until label styling lands).
    mpapp::label& heading(const std::string& text) {
        auto& l      = labels.add(body, text);
        l.font_bold  = true;
        l.font_size  = 15.0;
        l.text_color = tu_blue;
        return l;
    }
    mpapp::label& line(const std::string& text) {
        return labels.add(body, text);
    }
    void spacer() { labels.add(body, ""); }

    // Finish: bind native containers + wire the scroll/content chain.
    void end() {
        header.done();
        body.done();
        scroll.content = non_owning(body.as_view());
        root.vertical(0.0).add(header.as_view()).add(scroll);
        root.done();
        page.content = &root.as_view();
    }
};

// ---- Per-section content builders --------------------------------------

inline void build_information(section_page& p, const student& s) {
    p.heading("Студент: " + s.full_name);
    p.spacer();
    for (const auto& a : s.attributes)
        p.line(a.key + " " + a.value);
    p.spacer();
    p.line(s.group_note);
    p.spacer();

    // Announcements as a real CarouselView (one page per message). The
    // ◀ ▶ buttons drive carousel.scroll_to, which sets `position` and
    // switches the native page (GtkStack / FlipView / ViewFlipper).
    p.heading("Съобщения");
    p.opt_carousel = std::make_unique<mpapp::carousel_view>();
    p.opt_carousel->items_source = std::vector<std::string>{
        "Записването за летния семестър е отворено до 15-ти.",
        "Изпитната сесия започва на 1-ви юни.",
        "Стипендиите се изплащат до 10-то число на месеца.",
    };
    p.opt_carousel->loop = true;
    p.body.add(*p.opt_carousel);

    section_page* pp = &p;
    p.car_prev = std::make_unique<click_button>();
    p.car_next = std::make_unique<click_button>();
    p.car_prev->build("◀", [pp]() {
        if (pp->opt_carousel)
            pp->opt_carousel->scroll_to(pp->opt_carousel->position.get() - 1);
    });
    p.car_next->build("▶", [pp]() {
        if (pp->opt_carousel)
            pp->opt_carousel->scroll_to(pp->opt_carousel->position.get() + 1);
    });
    p.car_nav.horizontal(gap_md).add(p.car_prev->btn).add(p.car_next->btn);
    p.car_nav.done();
    p.body.add(p.car_nav.as_view());
    p.spacer();
    p.heading("Хронология на студентското състояние");
    for (const auto& r : s.status_history)
        p.line("Курс " + r.course + " · " + r.year + " · зав. сем. " +
               r.semester + " · " + r.state + " · " + r.reason +
               " · зап. №" + r.order_no + " от " + r.order_date);
}

inline void build_grades(section_page& p, const student& s) {
    p.line("Оценки по дисциплините от учебния Ви план.");
    p.spacer();
    for (const auto& g : s.grades) {
        p.heading(g.title);
        for (const auto& e : g.entries)
            p.line("   • " + e);
        p.spacer();
    }
}

inline void build_health(section_page& p, const student& s) {
    p.line(s.health_note);
}

inline void build_sport(section_page& p, const student& s) {
    p.line(s.sport_note);
}

inline void build_scholarship(section_page& p, const student& s) {
    p.line(s.scholarship_note);
}

inline void build_housing(section_page& p, const student& s) {
    p.line(s.housing_note);
}

inline void build_payments(section_page& p, const student& s) {
    p.line(s.payments_note);
    // Real picker for the payment kind (mirrors the portal's <select>).
    p.opt_picker = std::make_unique<mpapp::picker>();
    p.opt_picker->title = "Изберете вид плащане";
    p.opt_picker->items = s.payment_kinds;
    p.body.add(*p.opt_picker);
    p.picker_sel_lbl = &p.labels.add(p.body, "Избрано: (нищо)");
    // RFC-0014 command on the picker: executed on every selection change.
    section_page* pp = &p;
    p.opt_picker->command = std::make_shared<mpapp::relay_command>([pp]() {
        if (!pp->opt_picker || pp->picker_sel_lbl == nullptr) return;
        const int   idx = pp->opt_picker->selected_index.get();
        const auto& its = pp->opt_picker->items.get();
        pp->picker_sel_lbl->text =
            (idx >= 0 && idx < static_cast<int>(its.size()))
                ? ("Избрано: " + its[static_cast<std::size_t>(idx)])
                : std::string{"Избрано: (нищо)"};
    });
}

inline void build_identification(section_page& p, const student& s) {
    p.line(s.identification_note);
}

inline void build_history(section_page& p, const student& s) {
    p.heading("Последни влизания (от общо " +
              std::to_string(s.logins_total) + ")");
    for (const auto& r : s.logins)
        p.line(r.index + "  " + r.when + "  " + r.ip + "  " + r.agent);
    p.spacer();
    p.heading("Последни грешни опити (от общо " +
              std::to_string(s.failed_total) + ")");
    for (const auto& r : s.failed)
        p.line(r.index + "  " + r.when + "  " + r.ip + "  " + r.agent);
}

inline void build_help(section_page& p, const student& s) {
    p.line("Помощна информация");
    p.spacer();
    for (const auto& t : s.help) {
        p.heading(t.heading);
        p.line(t.body);
        p.spacer();
    }
}

// Dispatch a section to its builder.
inline void build_section(section s, section_page& p, const student& st) {
    switch (s) {
        case section::information:    build_information(p, st);    break;
        case section::health:         build_health(p, st);        break;
        case section::grades:         build_grades(p, st);        break;
        case section::sport:          build_sport(p, st);         break;
        case section::scholarship:    build_scholarship(p, st);   break;
        case section::housing:        build_housing(p, st);       break;
        case section::payments:       build_payments(p, st);      break;
        case section::identification: build_identification(p, st);break;
        case section::history:        build_history(p, st);       break;
        case section::help:           build_help(p, st);          break;
        default: break;
    }
}

// ---- Login page --------------------------------------------------------
//
// Faculty number + National ID (ЕГН). The demo authenticates when the
// faculty number matches the seeded student and the ID field is non-empty.
struct login_page {
    mpapp::page   page{};
    box           root{};
    mpapp::image  logo_{};
    mpapp::label  title_lbl{};
    mpapp::label  subtitle_lbl{};
    mpapp::label  fac_lbl{};
    mpapp::entry  fac_entry{};
    mpapp::label  id_lbl{};
    mpapp::entry  id_entry{};
    click_button  login_btn{};
    mpapp::label  status_lbl{};

    // RFC-0003 gesture demo: the subtitle is tappable (real on GTK/WinUI;
    // Android once its gesture handler lands).
    struct tap_cb_t {
        login_page* self;
        void operator()(const mpapp::tapped_event_args&) const {
            self->status_lbl.text = "Подзаглавието беше натиснато (жест).";
        }
    };
    tap_cb_t                                            tap_cb_{this};
    mpapp::signal_slot<const mpapp::tapped_event_args&> tap_slot_{};

    void build(std::function<void()> on_login) {
        page.title = "Вход";

        // Real image loader: the bundled TU logo (GtkPicture / BitmapImage /
        // BitmapFactory load it natively from the build-time asset path).
#ifdef UISS_ASSET_DIR
        logo_.source = std::string{UISS_ASSET_DIR} + "/tu_logo.png";
#endif
        logo_.aspect = mpapp::aspect_mode::aspect_fit;

        title_lbl.text       = "Технически университет - София";
        title_lbl.font_bold  = true;
        title_lbl.font_size  = 22.0;
        title_lbl.text_color = tu_blue;
        subtitle_lbl.text   = "Е-Студент — Информационна система за студенти";
        subtitle_lbl.font_size = 13.0;
        // RFC-0003: attach a tap recognizer to a non-button control.
        auto& tap = subtitle_lbl.add_gesture<mpapp::tap_gesture_recognizer>();
        tap.tapped.subscribe(tap_slot_, tap_cb_);
        fac_lbl.text      = "Факултетен номер:";
        fac_entry.placeholder = "напр. 201221001";
        fac_entry.semantic_description = "Факултетен номер";   // a11y name
        id_lbl.text       = "ЕГН (национален идентификатор):";
        id_entry.placeholder = "ЕГН";
        id_entry.is_password = true;
        id_entry.semantic_description = "ЕГН, национален идентификатор";   // a11y name
        status_lbl.text   = "";

        login_btn.build("Вход", std::move(on_login));

        root.vertical(gap_md, mpapp::thickness{pad_lg})
            .add(logo_)
            .add(title_lbl)
            .add(subtitle_lbl)
            .add(fac_lbl)
            .add(fac_entry)
            .add(id_lbl)
            .add(id_entry)
            .add(login_btn.btn)
            .add(status_lbl);
        root.done();

        page.content = &root.as_view();
    }
};

} // namespace uiss

#endif // UISS_PAGES_HPP
