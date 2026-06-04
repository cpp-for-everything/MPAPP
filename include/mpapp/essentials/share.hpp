// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::share` — native OS share sheet integration. Counterpart to
// MAUI Essentials `Share`. Supports sharing plain text/URLs via
// `share_text_request` and files via `share_file_request`. Abstract
// interface + an in-memory mock implementation whose recorded state is
// inspectable so tests can drive and verify share interactions. Real
// per-platform backends (Windows ShareContract, Android Intent.ACTION_SEND,
// iOS UIActivityViewController) implement the same interface and are
// injected via the DI container (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_SHARE_HPP
#define MPAPP_ESSENTIALS_SHARE_HPP

#include <optional>
#include <string>
#include <vector>

namespace mpapp {

// ---------------------------------------------------------------------------
// Request value types
// ---------------------------------------------------------------------------

// Mirrors MAUI's ShareTextRequest: title is shown in the share sheet,
// subject is used for email/messaging subjects, text is the body, and
// uri is an optional URL to include.
struct share_text_request {
    std::string title{};
    std::string subject{};
    std::string text{};
    std::string uri{};

    bool operator==(const share_text_request&) const = default;
};

// A single file entry for sharing. path is the file system path;
// content_type is a MIME type hint (e.g. "image/png", "text/plain").
struct share_file {
    std::string path{};
    std::string content_type{};

    bool operator==(const share_file&) const = default;
};

// Mirrors MAUI's ShareFileRequest: title is shown in the share sheet,
// files is the list of files to share.
struct share_file_request {
    std::string            title{};
    std::vector<share_file> files{};

    bool operator==(const share_file_request&) const = default;
};

// ---------------------------------------------------------------------------
// Abstract interface
// ---------------------------------------------------------------------------

class share {
public:
    virtual ~share() = default;

    // Present the OS share sheet for plain text / URL content.
    virtual void request(const share_text_request& req) = 0;

    // Present the OS share sheet for one or more files.
    virtual void request(const share_file_request& req) = 0;
};

// ---------------------------------------------------------------------------
// Mock / in-memory implementation
// ---------------------------------------------------------------------------
// Records each call so tests can inspect which overload was invoked and
// with what arguments. Provides request_count() across both overloads.

class mock_share final : public share {
public:
    mock_share() = default;

    void request(const share_text_request& req) override {
        last_text_request_ = req;
        ++request_count_;
    }

    void request(const share_file_request& req) override {
        last_file_request_ = req;
        ++request_count_;
    }

    // ---- Inspection helpers ------------------------------------------------

    // Returns the most recent share_text_request, or std::nullopt if
    // request(share_text_request) has never been called.
    [[nodiscard]] const std::optional<share_text_request>& last_text_request() const {
        return last_text_request_;
    }

    // Returns the most recent share_file_request, or std::nullopt if
    // request(share_file_request) has never been called.
    [[nodiscard]] const std::optional<share_file_request>& last_file_request() const {
        return last_file_request_;
    }

    // Returns the total number of times either request() overload was called.
    [[nodiscard]] int request_count() const noexcept {
        return request_count_;
    }

    // Reset all recorded state.
    void reset() noexcept {
        last_text_request_ = std::nullopt;
        last_file_request_ = std::nullopt;
        request_count_     = 0;
    }

private:
    std::optional<share_text_request> last_text_request_{};
    std::optional<share_file_request> last_file_request_{};
    int request_count_ = 0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_SHARE_HPP
