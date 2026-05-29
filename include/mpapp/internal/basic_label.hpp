// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Label.md
//
// `mpapp::basic_label` — text-display widget. T-0003 spike status: a thin
// cross-platform surface with a `text` Observable. The Windows handler
// wraps a `winrt::Microsoft::UI::Xaml::Controls::TextBlock`. The full
// MAUI Label surface lands in M-03.

#ifndef MPAPP_INTERNAL_BASIC_LABEL_HPP
#define MPAPP_INTERNAL_BASIC_LABEL_HPP

#include <string>

#include "../color.hpp"
#include "../control.hpp"
#include "../observable.hpp"
#include "../platform.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class label_handler;

class basic_label : public control<basic_label> {
public:
    basic_label() = default;

    basic_label(const basic_label&)            = delete;
    basic_label& operator=(const basic_label&) = delete;
    basic_label(basic_label&&)                 = delete;
    basic_label& operator=(basic_label&&)      = delete;

    Observable<std::string> text{""};

    // ----- Typography (RFC-0015 fidelity / goal: fonts loading) ----------
    // Minimal cross-platform font surface. `font_size` is in points;
    // 0 means "platform default". `font_bold` toggles weight.
    // `font_family` names a font; "" means the platform default family
    // (a registered RFC-0012 font alias resolves here once that lands).
    Observable<double>      font_size{0.0};
    Observable<bool>        font_bold{false};
    Observable<std::string> font_family{""};
    // Text color. Default is fully transparent (a == 0) = "unset" — real
    // handlers leave the platform default in place until a > 0.
    Observable<color>       text_color{color{0.0, 0.0, 0.0, 0.0}};

    label_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const label_handler<platform::current>& handler() const noexcept { return *handler_; }

    bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(label_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    label_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_INTERNAL_BASIC_LABEL_HPP
