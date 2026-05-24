// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Switch.md
//
// `mpapp::basic_switch_` — two-state toggle. The trailing underscore avoids the
// C++ `switch` keyword; the XAML element name remains `<Switch>` per the
// Known Differences row in Switch.md.

#ifndef MPAPP_INTERNAL_BASIC_SWITCH_HPP
#define MPAPP_INTERNAL_BASIC_SWITCH_HPP

#include "../command.hpp"
#include "../control.hpp"
#include "../observable.hpp"
#include "../platform.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class switch_handler;

class basic_switch_ : public control<basic_switch_> {
public:
    basic_switch_() = default;

    basic_switch_(const basic_switch_&)            = delete;
    basic_switch_& operator=(const basic_switch_&) = delete;
    basic_switch_(basic_switch_&&)                 = delete;
    basic_switch_& operator=(basic_switch_&&)      = delete;

    Observable<bool> is_on{false};

    void toggled(bool, Command<bool> = {}) {}

    switch_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const switch_handler<platform::current>& handler() const noexcept { return *handler_; }

    bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(switch_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    switch_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_INTERNAL_BASIC_SWITCH_HPP
