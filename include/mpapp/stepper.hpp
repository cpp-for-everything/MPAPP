// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Stepper.md
//
// `mpapp::stepper` — discrete numeric +/- input.

#ifndef MPAPP_STEPPER_HPP
#define MPAPP_STEPPER_HPP

#include "command.hpp"
#include "control.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform>
class stepper_handler;

class stepper : public control<stepper> {
public:
    stepper() = default;

    stepper(const stepper&)            = delete;
    stepper& operator=(const stepper&) = delete;
    stepper(stepper&&)                 = delete;
    stepper& operator=(stepper&&)      = delete;

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

#endif // MPAPP_STEPPER_HPP
