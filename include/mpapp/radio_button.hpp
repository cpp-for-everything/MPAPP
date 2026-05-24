// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/RadioButton.md
//
// `mpapp::radio_button` — single-selection toggle within a named group.
// Mock surface drops the `content` Observable<std::any> slot to keep the
// mock testable without dragging in `<any>` ostream support; the full
// surface lands with the typed-value batches.

#ifndef MPAPP_RADIO_BUTTON_HPP
#define MPAPP_RADIO_BUTTON_HPP

#include <string>

#include "command.hpp"
#include "control.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform = platform::current>
class radio_button_handler;

class radio_button : public control<radio_button> {
public:
    radio_button() = default;

    radio_button(const radio_button&)            = delete;
    radio_button& operator=(const radio_button&) = delete;
    radio_button(radio_button&&)                 = delete;
    radio_button& operator=(radio_button&&)      = delete;

    Observable<bool>        is_checked{false};
    Observable<std::string> group_name{""};

    void checked_changed(bool, Command<bool> = {}) {}

    radio_button_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const radio_button_handler<platform::current>& handler() const noexcept { return *handler_; }

    bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(radio_button_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    radio_button_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_RADIO_BUTTON_HPP
