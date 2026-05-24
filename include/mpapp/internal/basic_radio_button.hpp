// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/RadioButton.md
//
// `mpapp::basic_radio_button` — single-selection toggle within a named group.
// Mock surface drops the `content` Observable<std::any> slot to keep the
// mock testable without dragging in `<any>` ostream support; the full
// surface lands with the typed-value batches.

#ifndef MPAPP_INTERNAL_BASIC_RADIO_BUTTON_HPP
#define MPAPP_INTERNAL_BASIC_RADIO_BUTTON_HPP

#include <string>

#include "../command.hpp"
#include "../control.hpp"
#include "../observable.hpp"
#include "../platform.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class radio_button_handler;

class basic_radio_button : public control<basic_radio_button> {
public:
    basic_radio_button() = default;

    basic_radio_button(const basic_radio_button&)            = delete;
    basic_radio_button& operator=(const basic_radio_button&) = delete;
    basic_radio_button(basic_radio_button&&)                 = delete;
    basic_radio_button& operator=(basic_radio_button&&)      = delete;

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

#endif // MPAPP_INTERNAL_BASIC_RADIO_BUTTON_HPP
