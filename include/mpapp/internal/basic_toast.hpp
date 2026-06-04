// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. CommunityToolkit.Maui Toast surface (mock-first, P2).
//
// `mpapp::internal::basic_toast` — lightweight notification overlay.
// Mirrors CommunityToolkit.Maui's `Toast` control: text message,
// duration (short/long), visibility state, and show/dismiss commands.
// Emits `shown` / `dismissed` signals so callers can react without
// polling `is_shown`.

#ifndef MPAPP_INTERNAL_BASIC_TOAST_HPP
#define MPAPP_INTERNAL_BASIC_TOAST_HPP

#include <cstdint>
#include <string>
#include <string_view>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp {

// Duration hint passed to the platform toast layer.
// Mirrors CommunityToolkit.Maui ToastDuration (Short / Long).
enum class toast_duration : std::uint8_t {
    short_ = 0,
    long_  = 1,
};

constexpr std::string_view to_string(toast_duration d) noexcept {
    switch (d) {
        case toast_duration::short_: return "short";
        case toast_duration::long_:  return "long";
    }
    return "?";
}

} // namespace mpapp

namespace mpapp::internal {

// Primary template — concrete specialisations live in the per-platform
// handler headers. Forward-declared here so basic_toast can name it as
// a pointer-typed member without forcing a circular include.
template <class Platform = platform::current>
class toast_handler;

class basic_toast : public view {
public:
    basic_toast() = default;

    basic_toast(const basic_toast&)            = delete;
    basic_toast& operator=(const basic_toast&) = delete;
    basic_toast(basic_toast&&)                 = delete;
    basic_toast& operator=(basic_toast&&)      = delete;

    // ----- Properties -------------------------------------------------------
    Observable<std::string>    text{""};
    Observable<toast_duration> duration{toast_duration::short_};
    Observable<bool>           is_shown{false};

    // ----- Commands ---------------------------------------------------------
    // show() sets is_shown=true and emits `shown`.
    // dismiss() sets is_shown=false and emits `dismissed`.
    // Idempotent: repeated calls when already in the target state are no-ops.
    void show() {
        if (is_shown.get()) { return; }
        is_shown = true;
        shown.emit();
    }

    void dismiss() {
        if (!is_shown.get()) { return; }
        is_shown = false;
        dismissed.emit();
    }

    // ----- Signals ----------------------------------------------------------
    mpapp::signal<> shown;
    mpapp::signal<> dismissed;

    // ----- Handler attachment (pointer-based, opt-in) ----------------------
    toast_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const toast_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                    has_handler() const noexcept { return handler_ != nullptr; }
    void                                    set_handler(toast_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    toast_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal

#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>

template <>
struct std::formatter<mpapp::toast_duration> : std::formatter<std::string_view> {
    auto format(mpapp::toast_duration d, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(d), ctx);
    }
};

#endif // <format>

#endif // MPAPP_INTERNAL_BASIC_TOAST_HPP
