// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0021-tableview-cell-types.md
//
// `mpapp::entry_cell` — TableView row with a `label` + inline editable
// `text` field. Two-way bound; surfaces a `completed` signal when the
// user commits (Enter / loses focus).

#ifndef MPAPP_INTERNAL_BASIC_ENTRY_CELL_HPP
#define MPAPP_INTERNAL_BASIC_ENTRY_CELL_HPP

#include <cstdint>
#include <string>

#include "../cell.hpp"
#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"

namespace mpapp {

enum class keyboard_kind : std::uint8_t {
    default_  = 0,
    chat      = 1,
    email     = 2,
    numeric   = 3,
    telephone = 4,
    text      = 5,
    url       = 6,
};

} // namespace mpapp

namespace mpapp::internal {

template <class Platform = platform::current>
class entry_cell_handler;

class basic_entry_cell : public cell {
public:
    basic_entry_cell() = default;
    ~basic_entry_cell() override = default;

    basic_entry_cell(const basic_entry_cell&)            = delete;
    basic_entry_cell& operator=(const basic_entry_cell&) = delete;
    basic_entry_cell(basic_entry_cell&&)                 = delete;
    basic_entry_cell& operator=(basic_entry_cell&&)      = delete;

    Observable<std::string>   label{""};
    Observable<std::string>   text{""};
    Observable<std::string>   placeholder{""};
    Observable<keyboard_kind> keyboard{keyboard_kind::default_};

    signal<const std::string&> completed{};

    entry_cell_handler<platform::current>&       ec_handler() noexcept       { return *ec_handler_; }
    const entry_cell_handler<platform::current>& ec_handler() const noexcept { return *ec_handler_; }
    bool                                         has_ec_handler() const noexcept { return ec_handler_ != nullptr; }
    void                                         set_ec_handler(entry_cell_handler<platform::current>& h) noexcept { ec_handler_ = &h; }

private:
    entry_cell_handler<platform::current>* ec_handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_ENTRY_CELL_HPP
