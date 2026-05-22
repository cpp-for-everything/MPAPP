// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TableView.md
//
// `mpapp::table_view` — static section/row list. Two parallel surfaces:
//
//   1. `sections`        — title + vector<string> rows. Flat-string
//                          rendering. Useful for tests and quick
//                          static lists.
//   2. `typed_sections`  — title + vector<cell*> rows. Each cell is a
//                          fully-typed row from the cell tree
//                          (text_cell / entry_cell / switch_cell /
//                          view_cell / image_cell). The handler
//                          renders each cell via its own native widget
//                          through ADR-0013 dispatch. Cells are
//                          non-owning — the app owns the cell
//                          lifetime, the table_view just references.
//
// When `typed_sections` is non-empty, the typed path wins. Setting
// both is supported only as a transitional convenience.

#ifndef MPAPP_TABLE_VIEW_HPP
#define MPAPP_TABLE_VIEW_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp {

enum class table_intent : std::uint8_t {
    data     = 0,
    menu     = 1,
    settings = 2,
    form     = 3,
};

struct table_section_data {
    std::string              title{};
    std::vector<std::string> rows{};

    bool operator==(const table_section_data&) const = default;
};

// Forward-declare cell so this header doesn't have to pull in the full
// cell tree.
class cell;

struct table_section_typed {
    std::string         title{};
    std::vector<cell*>  rows{};   // non-owning — app owns each cell

    bool operator==(const table_section_typed&) const = default;
};

template <class Platform>
class table_view_handler;

class table_view : public view {
public:
    table_view() = default;
    ~table_view() override = default;

    table_view(const table_view&)            = delete;
    table_view& operator=(const table_view&) = delete;
    table_view(table_view&&)                 = delete;
    table_view& operator=(table_view&&)      = delete;

    // ----- Surface ------------------------------------------------------

    Observable<std::vector<table_section_data>>  sections{};
    Observable<std::vector<table_section_typed>> typed_sections{};
    Observable<table_intent>                     intent{table_intent::data};
    Observable<int>                              row_height{-1};       // -1 = native default
    Observable<bool>                             has_unevenly_sized_rows{false};

    // ----- Events -------------------------------------------------------

    signal<int /*section*/, int /*row*/> row_tapped{};

    // ----- Helpers ------------------------------------------------------

    std::size_t total_row_count() const noexcept {
        std::size_t n = 0;
        for (const auto& s : sections.get()) n += s.rows.size();
        return n;
    }

    void add_section(const std::string& title) {
        auto v = sections.get();
        v.push_back(table_section_data{title, {}});
        sections.set(std::move(v));
    }

    void add_row(std::size_t section_index, const std::string& row) {
        auto v = sections.get();
        if (section_index >= v.size()) return;
        v[section_index].rows.push_back(row);
        sections.set(std::move(v));
    }

    // ----- Handler ------------------------------------------------------

    table_view_handler<platform::current>&       tv_handler() noexcept       { return *tv_handler_; }
    const table_view_handler<platform::current>& tv_handler() const noexcept { return *tv_handler_; }
    bool                                         has_tv_handler() const noexcept { return tv_handler_ != nullptr; }
    void                                         set_tv_handler(table_view_handler<platform::current>& h) noexcept { tv_handler_ = &h; }

private:
    table_view_handler<platform::current>* tv_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_TABLE_VIEW_HPP
