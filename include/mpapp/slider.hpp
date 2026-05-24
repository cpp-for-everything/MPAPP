// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Slider.md
//
// `mpapp::slider` — continuous-range double input. The mock surface
// exposes the primitive-typed slots (`value`, `minimum`, `maximum`); the
// color / image_source slots land with the typed-value batches.

#ifndef MPAPP_SLIDER_HPP
#define MPAPP_SLIDER_HPP

#include "command.hpp"
#include "control.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform = platform::current>
class slider_handler;

class slider : public control<slider> {
public:
    slider() = default;

    slider(const slider&)            = delete;
    slider& operator=(const slider&) = delete;
    slider(slider&&)                 = delete;
    slider& operator=(slider&&)      = delete;

    Observable<double> value{0.0};
    Observable<double> minimum{0.0};
    Observable<double> maximum{1.0};

    void value_changed(double, Command<double> = {}) {}
    void drag_started(Command<> = {}) {}
    void drag_completed(Command<> = {}) {}

    slider_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const slider_handler<platform::current>& handler() const noexcept { return *handler_; }

    bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(slider_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    slider_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_SLIDER_HPP
