// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0021-tableview-cell-types.md
//
// `mpapp::text_cell` — TableView row showing primary `text` + optional
// `detail` text. Both render with native row styling on each platform
// (iOS Settings-style rows, Android list items, etc.).

#ifndef MPAPP_INTERNAL_BASIC_TEXT_CELL_HPP
#define MPAPP_INTERNAL_BASIC_TEXT_CELL_HPP

#include <string>

#include "../cell.hpp"
#include "../observable.hpp"
#include "../platform.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class text_cell_handler;

class basic_text_cell : public cell {
public:
    basic_text_cell() = default;
    ~basic_text_cell() override = default;

    basic_text_cell(const basic_text_cell&)            = delete;
    basic_text_cell& operator=(const basic_text_cell&) = delete;
    basic_text_cell(basic_text_cell&&)                 = delete;
    basic_text_cell& operator=(basic_text_cell&&)      = delete;

    Observable<std::string> text{""};
    Observable<std::string> detail{""};
    Observable<std::string> text_color{""};
    Observable<std::string> detail_color{""};

    text_cell_handler<platform::current>&       tc_handler() noexcept       { return *tc_handler_; }
    const text_cell_handler<platform::current>& tc_handler() const noexcept { return *tc_handler_; }
    bool                                        has_tc_handler() const noexcept { return tc_handler_ != nullptr; }
    void                                        set_tc_handler(text_cell_handler<platform::current>& h) noexcept { tc_handler_ = &h; }

private:
    text_cell_handler<platform::current>* tc_handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_TEXT_CELL_HPP
