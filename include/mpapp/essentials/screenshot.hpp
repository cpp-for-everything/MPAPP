// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::screenshot` — screen-capture API. Counterpart to MAUI Essentials
// `Screenshot`. Abstract interface + an in-memory mock whose canned result
// and supported flag are fully test-settable. Real per-platform backends
// (Windows Graphics.Capture, Linux XCB/Wayland, Android MediaProjection,
// iOS UIGraphics) implement the same interface and are injected via the DI
// container (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_SCREENSHOT_HPP
#define MPAPP_ESSENTIALS_SCREENSHOT_HPP

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace mpapp {

// Image encoding used for the captured bytes — mirrors MAUI ScreenshotFormat.
enum class screenshot_format : std::uint8_t {
    png  = 0,
    jpeg = 1,
};

[[nodiscard]] constexpr std::string_view to_string(screenshot_format f) noexcept {
    switch (f) {
        case screenshot_format::png:  return "png";
        case screenshot_format::jpeg: return "jpeg";
        default:                      return "?";
    }
}

// Payload returned by a successful capture.
struct screenshot_result {
    int                        width  = 0;
    int                        height = 0;
    std::vector<std::uint8_t>  bytes{};
    screenshot_format          format = screenshot_format::png;

    bool operator==(const screenshot_result&) const = default;
};

// Abstract interface.
class screenshot {
public:
    virtual ~screenshot() = default;

    // Returns true when the current platform supports screen capture.
    [[nodiscard]] virtual bool is_captured_supported() const = 0;

    // Captures the screen. Returns std::nullopt when not supported or when
    // the capture fails (e.g. the user denies the permission prompt).
    [[nodiscard]] virtual std::optional<screenshot_result> capture() = 0;
};

// Mock / in-memory implementation.
//
// * `set_captured_supported(bool)` controls whether is_captured_supported()
//   returns true and whether capture() produces a result.
// * `set_result(screenshot_result)` seeds the canned value returned by
//   capture() when the mock is in supported mode.
// * `capture_count()` exposes how many times capture() has been called so
//   tests can assert invocation counts without side effects.
// * When not supported, capture() returns std::nullopt regardless of the
//   canned result.
class mock_screenshot final : public screenshot {
public:
    // Default-construct: supported, empty-byte result, capture count zero.
    explicit mock_screenshot(bool supported = true) noexcept
        : supported_{ supported } {}

    // ---- screenshot interface ---------------------------------------------

    [[nodiscard]] bool is_captured_supported() const override {
        return supported_;
    }

    [[nodiscard]] std::optional<screenshot_result> capture() override {
        ++capture_count_;
        if (!supported_) {
            return std::nullopt;
        }
        return result_;
    }

    // ---- Test-control helpers --------------------------------------------

    void set_captured_supported(bool s) noexcept { supported_ = s; }

    void set_result(const screenshot_result& r) { result_ = r; }

    [[nodiscard]] int capture_count() const noexcept { return capture_count_; }

private:
    bool              supported_     = true;
    screenshot_result result_{};
    int               capture_count_ = 0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_SCREENSHOT_HPP
