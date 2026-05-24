// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/CheckBox.md
//
// `mpapp::basic_check_box` — two-state opt-in selector.

#ifndef MPAPP_INTERNAL_BASIC_CHECK_BOX_HPP
#define MPAPP_INTERNAL_BASIC_CHECK_BOX_HPP

#include "../command.hpp"
#include "../control.hpp"
#include "../observable.hpp"
#include "../platform.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class check_box_handler;

class basic_check_box : public control<basic_check_box> {
public:
    basic_check_box() = default;

    basic_check_box(const basic_check_box&)            = delete;
    basic_check_box& operator=(const basic_check_box&) = delete;
    basic_check_box(basic_check_box&&)                 = delete;
    basic_check_box& operator=(basic_check_box&&)      = delete;

    Observable<bool> is_checked{false};

    void checked_changed(bool, Command<bool> = {}) {}

    check_box_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const check_box_handler<platform::current>& handler() const noexcept { return *handler_; }

    bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(check_box_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    check_box_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_INTERNAL_BASIC_CHECK_BOX_HPP
