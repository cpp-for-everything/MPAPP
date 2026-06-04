// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::sms` — compose and send SMS messages. Counterpart to MAUI
// Essentials `Sms`. Abstract interface + a mock implementation whose
// recorded state (last composed message, call count) is inspectable in
// tests. Real per-platform backends (Windows, Android, iOS) implement
// the same interface and are injected via the DI container (RFC-0011).
// No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_SMS_HPP
#define MPAPP_ESSENTIALS_SMS_HPP

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace mpapp {

// Message value type: mirrors MAUI's SmsMessage.
struct sms_message {
    std::string              body{};
    std::vector<std::string> recipients{};

    bool operator==(const sms_message&) const = default;
};

// Abstract SMS composer interface.
class sms {
public:
    virtual ~sms() = default;

    // Open the platform SMS composer with no pre-filled content.
    virtual void compose() = 0;

    // Open the platform SMS composer pre-filled with `message`.
    virtual void compose(const sms_message& message) = 0;
};

// Mock implementation: records every compose() call so tests can inspect it.
// `is_supported_` controls whether compose() throws or records; default is
// supported. Use `set_supported(false)` to test the not-supported path.
class mock_sms final : public sms {
public:
    mock_sms() = default;

    // Control whether compose() succeeds (true) or throws std::runtime_error
    // to simulate an unsupported platform (false).
    void set_supported(bool supported) noexcept { supported_ = supported; }
    [[nodiscard]] bool is_supported() const noexcept { return supported_; }

    // compose() with no content — records an empty sms_message.
    void compose() override {
        if (!supported_) {
            throw_not_supported();
        }
        last_message_ = sms_message{};
        ++compose_count_;
    }

    // compose() with a pre-filled message.
    void compose(const sms_message& message) override {
        if (!supported_) {
            throw_not_supported();
        }
        last_message_ = message;
        ++compose_count_;
    }

    // Inspection API — nodiscard so callers do not silently discard the value.
    [[nodiscard]] std::optional<sms_message> last_message() const noexcept {
        return last_message_;
    }
    [[nodiscard]] std::size_t compose_count() const noexcept {
        return compose_count_;
    }

    // Reset recorded state (does not affect supported_ flag).
    void reset() noexcept {
        last_message_ = std::nullopt;
        compose_count_ = 0;
    }

private:
    [[noreturn]] static void throw_not_supported() {
        throw std::runtime_error("sms: not supported on this platform");
    }

    bool                      supported_     = true;
    std::optional<sms_message> last_message_  = std::nullopt;
    std::size_t               compose_count_ = 0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_SMS_HPP
