// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Stepper.md
//
// `mpapp::basic_stepper` — discrete numeric +/- input.

#ifndef MPAPP_INTERNAL_BASIC_STEPPER_HPP
#define MPAPP_INTERNAL_BASIC_STEPPER_HPP

#include "../command.hpp"
#include "../control.hpp"
#include "../observable.hpp"
#include "../platform.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class stepper_handler;

class basic_stepper : public control<basic_stepper> {
public:
    basic_stepper() = default;

    basic_stepper(const basic_stepper&)            = delete;
    basic_stepper& operator=(const basic_stepper&) = delete;
    basic_stepper(basic_stepper&&)                 = delete;
    basic_stepper& operator=(basic_stepper&&)      = delete;

    Observable<double> value{0.0};
    Observable<double> minimum{0.0};
    Observable<double> maximum{100.0};
    Observable<double> interval{1.0};

    void value_changed(double, Command<double> = {}) {}

    stepper_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const stepper_handler<platform::current>& handler() const noexcept { return *handler_; }

    bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(stepper_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    stepper_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_INTERNAL_BASIC_STEPPER_HPP
