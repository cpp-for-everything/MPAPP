// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Switch.md
//
// `mpapp::switch_` — two-state toggle. The trailing underscore avoids the
// C++ `switch` keyword; the XAML element name remains `<Switch>` per the
// Known Differences row in Switch.md.

#ifndef MPAPP_SWITCH_HPP
#define MPAPP_SWITCH_HPP

#include "command.hpp"
#include "control.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform = platform::current>
class switch_handler;

class switch_ : public control<switch_> {
public:
    switch_() = default;

    switch_(const switch_&)            = delete;
    switch_& operator=(const switch_&) = delete;
    switch_(switch_&&)                 = delete;
    switch_& operator=(switch_&&)      = delete;

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

#endif // MPAPP_SWITCH_HPP
