// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/CheckBox.md
//
// `mpapp::check_box` — two-state opt-in selector.

#ifndef MPAPP_CHECK_BOX_HPP
#define MPAPP_CHECK_BOX_HPP

#include "command.hpp"
#include "control.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform = platform::current>
class check_box_handler;

class check_box : public control<check_box> {
public:
    check_box() = default;

    check_box(const check_box&)            = delete;
    check_box& operator=(const check_box&) = delete;
    check_box(check_box&&)                 = delete;
    check_box& operator=(check_box&&)      = delete;

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

#endif // MPAPP_CHECK_BOX_HPP
