// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::email` — compose-and-send email integration. Counterpart to MAUI
// Essentials `Email`. Supports composing messages with To/CC/BCC recipient
// lists, subject, plain-text or HTML body, and file attachments. Abstract
// interface + an in-memory mock implementation whose recorded state is
// inspectable so tests can drive and verify email compose interactions. Real
// per-platform backends (Windows mailto: URI / MAPI, Linux xdg-email,
// Android Intent.ACTION_SENDTO, iOS MFMailComposeViewController) implement
// the same interface and are injected via the DI container (RFC-0011).
// No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_EMAIL_HPP
#define MPAPP_ESSENTIALS_EMAIL_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mpapp {

// ---------------------------------------------------------------------------
// Enum: email body format
// ---------------------------------------------------------------------------

// Mirrors MAUI EmailBodyFormat.
enum class email_body_format : std::uint8_t {
    plain_text = 0,
    html       = 1,
};

[[nodiscard]] constexpr std::string_view to_string(email_body_format f) noexcept {
    switch (f) {
        case email_body_format::plain_text: return "plain_text";
        case email_body_format::html:       return "html";
        default:                            return "?";
    }
}

// ---------------------------------------------------------------------------
// Value type: email_message
// ---------------------------------------------------------------------------

// Mirrors MAUI EmailMessage. All fields are optional from the platform's
// perspective; the mock records whatever is provided.
struct email_message {
    std::vector<std::string> to{};
    std::vector<std::string> cc{};
    std::vector<std::string> bcc{};
    std::string              subject{};
    std::string              body{};
    email_body_format        format      = email_body_format::plain_text;
    std::vector<std::string> attachments{};

    bool operator==(const email_message&) const = default;
};

// ---------------------------------------------------------------------------
// Abstract interface
// ---------------------------------------------------------------------------

class email {
public:
    virtual ~email() = default;

    // Open the platform email composer with no pre-filled content.
    virtual void compose() = 0;

    // Open the platform email composer pre-filled with `message`.
    virtual void compose(const email_message& message) = 0;
};

// ---------------------------------------------------------------------------
// Mock / in-memory implementation
// ---------------------------------------------------------------------------
// Records each compose call so tests can inspect which overload was invoked
// and with what arguments. Tracks compose_count() across both overloads.

class mock_email final : public email {
public:
    mock_email() = default;

    // ---- email interface implementation ------------------------------------

    // Records a blank-compose call (no message). last_message() is NOT
    // updated so callers can distinguish a blank compose from a message
    // compose that was called with a default-constructed email_message.
    void compose() override {
        ++compose_count_;
        ++blank_compose_count_;
    }

    // Records a message compose call. Updates last_message().
    void compose(const email_message& message) override {
        last_message_ = message;
        ++compose_count_;
    }

    // ---- Inspection helpers -----------------------------------------------

    // Returns the most recent email_message passed to compose(message), or
    // std::nullopt if that overload has never been called.
    [[nodiscard]] const std::optional<email_message>& last_message() const noexcept {
        return last_message_;
    }

    // Returns the total number of times either compose() overload was called.
    [[nodiscard]] int compose_count() const noexcept {
        return compose_count_;
    }

    // Returns the number of times the no-argument compose() overload was called.
    [[nodiscard]] int blank_compose_count() const noexcept {
        return blank_compose_count_;
    }

    // Reset all recorded state.
    void reset() noexcept {
        last_message_       = std::nullopt;
        compose_count_      = 0;
        blank_compose_count_ = 0;
    }

private:
    std::optional<email_message> last_message_{};
    int compose_count_       = 0;
    int blank_compose_count_ = 0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_EMAIL_HPP
