// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. УИСС demo — page composition.
//
// One reusable `section_page` (header bar with a ≡ flyout toggle + title,
// over a scrollable content column) plus the `login_page` and the ten
// per-section content builders. Every builder binds to a single
// `uiss::student` record — no platform code, no ifdefs.

#ifndef UISS_PAGES_HPP
#define UISS_PAGES_HPP

#include <functional>
#include <string>

#include <mpapp/entry.hpp>
#include <mpapp/label.hpp>
#include <mpapp/page.hpp>
#include <mpapp/scroll_view.hpp>

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

    // Begin a page: header bar + an empty content column ready for rows.
    void begin(const std::string& title, std::function<void()> on_menu) {
        title_lbl.text = title;
        menu_btn.build("≡", std::move(on_menu));   // ≡
        header.horizontal(gap_md, mpapp::thickness{pad_md})
              .add(menu_btn.btn)
              .add(title_lbl);
        body.vertical(gap_sm, mpapp::thickness{pad_lg});
    }

    // A bold-ish section heading (plain text until label styling lands).
    mpapp::label& heading(const std::string& text) {
        return labels.add(body, text);
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
    for (const auto& k : s.payment_kinds)
        p.line("   • " + k);
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
    mpapp::label  title_lbl{};
    mpapp::label  subtitle_lbl{};
    mpapp::label  fac_lbl{};
    mpapp::entry  fac_entry{};
    mpapp::label  id_lbl{};
    mpapp::entry  id_entry{};
    click_button  login_btn{};
    mpapp::label  status_lbl{};

    void build(std::function<void()> on_login) {
        page.title = "Вход";

        title_lbl.text    = "Технически университет - София";
        subtitle_lbl.text = "Е-Студент — Информационна система за студенти";
        fac_lbl.text      = "Факултетен номер:";
        fac_entry.placeholder = "напр. 201221001";
        id_lbl.text       = "ЕГН (национален идентификатор):";
        id_entry.placeholder = "ЕГН";
        id_entry.is_password = true;
        status_lbl.text   = "";

        login_btn.build("Вход", std::move(on_login));

        root.vertical(gap_md, mpapp::thickness{pad_lg})
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
