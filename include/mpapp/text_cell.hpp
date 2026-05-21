// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0021-tableview-cell-types.md
//
// `mpapp::text_cell` — TableView row showing primary `text` + optional
// `detail` text. Both render with native row styling on each platform
// (iOS Settings-style rows, Android list items, etc.).

#ifndef MPAPP_TEXT_CELL_HPP
#define MPAPP_TEXT_CELL_HPP

#include <string>

#include "cell.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform>
class text_cell_handler;

class text_cell : public cell {
public:
    text_cell() = default;
    ~text_cell() override = default;

    text_cell(const text_cell&)            = delete;
    text_cell& operator=(const text_cell&) = delete;
    text_cell(text_cell&&)                 = delete;
    text_cell& operator=(text_cell&&)      = delete;

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

} // namespace mpapp

#endif // MPAPP_TEXT_CELL_HPP
