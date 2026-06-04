// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::phone_dialer` — place a phone call by opening the platform dialer.
// Counterpart to MAUI Essentials `PhoneDialer`. Abstract interface + a mock
// implementation whose recorded state (last dialled number, call count) is
// inspectable in tests. Real per-platform backends (Windows Phone Link,
// Android Intent.ACTION_DIAL, iOS UIApplication.openURL) implement the same
// interface and are injected via the DI container (RFC-0011). No macros;
// header-only interface.

#ifndef MPAPP_ESSENTIALS_PHONE_DIALER_HPP
#define MPAPP_ESSENTIALS_PHONE_DIALER_HPP

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>

namespace mpapp {

// Abstract phone-dialer interface.
class phone_dialer {
public:
    virtual ~phone_dialer() = default;

    // Returns true when the platform provides a native phone-dialling
    // capability (e.g. Android/iOS phone, or Windows with Phone Link).
    [[nodiscard]] virtual bool is_supported() const = 0;

    // Open the platform dialler pre-filled with `number`.
    // Implementations should throw std::runtime_error when !is_supported().
    virtual void open(const std::string& number) = 0;
};

// Mock implementation: `is_supported_` is settable (default true); every
// call to `open()` records the number in `last_number_` and increments
// `open_count_`. When `is_supported_` is false, `open()` throws
// std::runtime_error to simulate an unsupported platform.
class mock_phone_dialer final : public phone_dialer {
public:
    mock_phone_dialer() = default;

    // Control whether the platform supports phone dialling.
    void set_supported(bool supported) noexcept { supported_ = supported; }

    // ---- phone_dialer interface -----------------------------------------------

    [[nodiscard]] bool is_supported() const override { return supported_; }

    void open(const std::string& number) override {
        if (!supported_) {
            throw std::runtime_error("phone_dialer: not supported on this platform");
        }
        last_number_ = number;
        ++open_count_;
    }

    // ---- Inspection API -------------------------------------------------------

    // The number passed to the most recent successful open() call, or nullopt
    // if open() has never been called (or every call threw).
    [[nodiscard]] std::optional<std::string> last_number() const noexcept {
        return last_number_;
    }

    // Total number of successful open() calls.
    [[nodiscard]] std::size_t open_count() const noexcept { return open_count_; }

    // Reset recorded state without altering the supported flag.
    void reset() noexcept {
        last_number_ = std::nullopt;
        open_count_  = 0;
    }

private:
    bool                       supported_   = true;
    std::optional<std::string> last_number_ = std::nullopt;
    std::size_t                open_count_  = 0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_PHONE_DIALER_HPP
