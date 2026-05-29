// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. УИСС demo — the application.
//
// Single codebase, no ifdefs. The same composition runs on Windows
// (WinUI 3), Linux (GTK4) and Android (NDK); `mpapp::run<uiss_app>`
// picks the platform handler set at link time.
//
// Structure:
//   window → flyout_page
//              ├── flyout: nav menu (10 sections + logout)
//              └── detail: login_page  →  one of 10 section_pages
//
// Login (faculty number + ЕГН) swaps the detail to Информация and opens
// the menu; each section's ≡ toggles the flyout; logout returns to login.

#ifndef UISS_APP_HPP
#define UISS_APP_HPP

#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include <mpapp/application.hpp>
#include <mpapp/flyout_page.hpp>
#include <mpapp/label.hpp>
#include <mpapp/page.hpp>
#include <mpapp/window.hpp>

#include "data.hpp"
#include "pages.hpp"
#include "support.hpp"

namespace uiss {

inline constexpr std::size_t kSections =
    static_cast<std::size_t>(section::count);

class uiss_app : public mpapp::application {
public:
    void on_launch() override {
        student_ = seed();

        build_menu();
        build_login();
        build_sections();

        // Root master/detail container.
        fp_.flyout       = &menu_page_;
        fp_.detail       = &login_.page;
        fp_.is_presented = false;        // hidden until authenticated

        window_.title  = "УИСС — Е-Студент (MPAPP)";
        window_.width  = 940;
        window_.height = 680;
        window_.content = &fp_;
        window_.show();
    }

private:
    // ----- Builders -----------------------------------------------------

    void build_menu() {
        menu_page_.title = "Меню";

        uni_lbl_.text       = "Технически университет - София";
        uni_lbl_.font_bold  = true;
        uni_lbl_.font_size  = 15.0;
        uni_lbl_.text_color = tu_blue;
        product_lbl_.text   = "Е-Студент";
        product_lbl_.font_size = 13.0;
        student_lbl_.text   = "Студент: " + student_.full_name;
        student_lbl_.font_bold = true;

        menu_box_.vertical(gap_sm, mpapp::thickness{pad_md})
                 .add(uni_lbl_)
                 .add(product_lbl_)
                 .add(student_lbl_);

        for (std::size_t i = 0; i < kSections; ++i) {
            auto b = std::make_unique<click_button>();
            const auto sec = static_cast<section>(i);
            b->build(section_title(sec),
                     [this, i]() { navigate(static_cast<section>(i)); });
            menu_box_.add(b->btn);
            nav_buttons_[i] = std::move(b);
        }

        logout_btn_.build("Изход", [this]() { logout(); });
        menu_box_.add(logout_btn_.btn);

        menu_box_.done();
        menu_page_.content = &menu_box_.as_view();
    }

    void build_login() {
        login_.build([this]() { try_login(); });
    }

    void build_sections() {
        for (std::size_t i = 0; i < kSections; ++i) {
            const auto sec = static_cast<section>(i);
            auto& p = sections_[i];
            p.begin(section_title(sec), [this]() { fp_.toggle(); });
            build_section(sec, p, student_);
            p.end();
        }
    }

    // ----- Behaviour ----------------------------------------------------

    void try_login() {
        const std::string fac = login_.fac_entry.text.get();
        const std::string id  = login_.id_entry.text.get();
        if (fac == student_.faculty_number && !id.empty()) {
            logged_in_ = true;
            login_.status_lbl.text = "";
            navigate(section::information);
            fp_.present();    // reveal the nav menu
        } else {
            login_.status_lbl.text =
                "Невалиден факултетен номер или ЕГН. Опитайте отново.";
        }
    }

    void navigate(section s) {
        if (!logged_in_) return;
        const auto idx = static_cast<std::size_t>(s);
        fp_.detail = &sections_[idx].page;
        fp_.dismiss();   // close the flyout after a pick
    }

    void logout() {
        logged_in_ = false;
        login_.fac_entry.text = "";
        login_.id_entry.text  = "";
        login_.status_lbl.text = "";
        fp_.detail = &login_.page;
        fp_.dismiss();
    }

    // ----- State --------------------------------------------------------

    student student_{};
    bool     logged_in_ = false;

    mpapp::window      window_{};
    mpapp::flyout_page fp_{};

    // Flyout (nav) pane.
    mpapp::page  menu_page_{};
    box          menu_box_{};
    mpapp::label uni_lbl_{};
    mpapp::label product_lbl_{};
    mpapp::label student_lbl_{};
    std::array<std::unique_ptr<click_button>, kSections> nav_buttons_{};
    click_button logout_btn_{};

    // Detail pages.
    login_page                            login_{};
    std::array<section_page, kSections>   sections_{};
};

} // namespace uiss

#endif // UISS_APP_HPP
