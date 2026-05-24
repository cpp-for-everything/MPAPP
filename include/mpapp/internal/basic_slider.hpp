// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Slider.md
//
// `mpapp::basic_slider` — continuous-range double input. The mock surface
// exposes the primitive-typed slots (`value`, `minimum`, `maximum`); the
// color / image_source slots land with the typed-value batches.

#ifndef MPAPP_INTERNAL_BASIC_SLIDER_HPP
#define MPAPP_INTERNAL_BASIC_SLIDER_HPP

#include "../command.hpp"
#include "../control.hpp"
#include "../observable.hpp"
#include "../platform.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class slider_handler;

class basic_slider : public control<basic_slider> {
public:
    basic_slider() = default;

    basic_slider(const basic_slider&)            = delete;
    basic_slider& operator=(const basic_slider&) = delete;
    basic_slider(basic_slider&&)                 = delete;
    basic_slider& operator=(basic_slider&&)      = delete;

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

#endif // MPAPP_INTERNAL_BASIC_SLIDER_HPP
