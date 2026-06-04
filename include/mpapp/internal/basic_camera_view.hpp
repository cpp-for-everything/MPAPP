// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/CameraView.md
//
// `mpapp::internal::basic_camera_view` — CommunityToolkit CameraView surface.
// Mock surface (P2). Mirrors .NET MAUI CommunityToolkit CameraView:
// flash, position, availability, torch, zoom, preview controls, capture.

#ifndef MPAPP_INTERNAL_BASIC_CAMERA_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_CAMERA_VIEW_HPP

#include <cstdint>
#include <string>
#include <string_view>

#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_CAMERA_VIEW_HAS_STD_FORMAT 1
#endif

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp {

// Flash mode mirrors CommunityToolkit CameraView.CameraFlashMode.
enum class camera_flash : std::uint8_t {
    off   = 0,
    on    = 1,
    auto_ = 2,
};

constexpr std::string_view to_string(camera_flash f) noexcept {
    switch (f) {
        case camera_flash::off:   return "off";
        case camera_flash::on:    return "on";
        case camera_flash::auto_: return "auto";
    }
    return "?";
}

// Camera lens position mirrors CommunityToolkit CameraView.CameraFacing.
enum class camera_position : std::uint8_t {
    front = 0,
    rear  = 1,
};

constexpr std::string_view to_string(camera_position p) noexcept {
    switch (p) {
        case camera_position::front: return "front";
        case camera_position::rear:  return "rear";
    }
    return "?";
}

} // namespace mpapp

namespace mpapp::internal {

template <class Platform = platform::current>
class camera_view_handler;

class basic_camera_view : public view {
public:
    basic_camera_view() = default;
    ~basic_camera_view() override = default;

    basic_camera_view(const basic_camera_view&)            = delete;
    basic_camera_view& operator=(const basic_camera_view&) = delete;
    basic_camera_view(basic_camera_view&&)                 = delete;
    basic_camera_view& operator=(basic_camera_view&&)      = delete;

    // ----- Properties -------------------------------------------------------

    // Active flash mode (CameraView.CameraFlashMode).
    Observable<camera_flash>    flash{camera_flash::off};
    // Which lens to use (front / rear). CameraView.CameraFacing.
    Observable<camera_position> position{camera_position::rear};
    // Whether a usable camera is present. Read-only on real platforms;
    // settable here so mock tests can exercise both branches.
    Observable<bool>            is_available{false};
    // Torch (flashlight) on/off. CameraView.TorchOn.
    Observable<bool>            is_torch_on{false};
    // Zoom factor. 1.0 = normal. CameraView.ZoomFactor.
    Observable<double>          zoom_factor{1.0};

    // ----- Signals ----------------------------------------------------------

    // Emitted after a successful capture; carries the platform file path.
    // CommunityToolkit: MediaCaptured event.
    signal<std::string>  media_captured{};
    // Emitted when the camera subsystem encounters an error; carries a
    // descriptive message. CommunityToolkit: CameraError event.
    signal<std::string>  camera_error{};

    // ----- Commands ---------------------------------------------------------

    // Trigger a still/video capture. Mock implementation: emits
    // camera_error when is_available is false; otherwise emits
    // media_captured with `next_capture_path_`. Real handlers override
    // via the per-platform handler wire-up (not yet implemented).
    void capture() {
        if (!is_available.get()) {
            camera_error.emit(std::string{"camera not available"});
            return;
        }
        media_captured.emit(next_capture_path_);
    }

    // Start the camera preview stream. Mock: emits preview_started signal.
    void start_preview() {
        preview_started.emit();
    }

    // Stop the camera preview stream. Mock: emits preview_stopped signal.
    void stop_preview() {
        preview_stopped.emit();
    }

    // Preview lifecycle signals (subscribed by the mock handler).
    signal<> preview_started{};
    signal<> preview_stopped{};

    // ----- Mock helpers -----------------------------------------------------

    // Set the file path that capture() will supply to media_captured.
    void set_next_capture_path(std::string path) {
        next_capture_path_ = std::move(path);
    }

    [[nodiscard]] const std::string& next_capture_path() const noexcept {
        return next_capture_path_;
    }

    // ----- Handler ----------------------------------------------------------

    camera_view_handler<platform::current>&       handler() noexcept
        { return *handler_; }
    const camera_view_handler<platform::current>& handler() const noexcept
        { return *handler_; }
    bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(camera_view_handler<platform::current>& h) noexcept
        { handler_ = &h; }

private:
    std::string                              next_capture_path_{"captured.jpg"};
    camera_view_handler<platform::current>*  handler_ = nullptr;
};

} // namespace mpapp::internal


#ifdef MPAPP_CAMERA_VIEW_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::camera_flash> : std::formatter<std::string_view> {
    auto format(mpapp::camera_flash f, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(f), ctx);
    }
};

template <>
struct std::formatter<mpapp::camera_position> : std::formatter<std::string_view> {
    auto format(mpapp::camera_position p, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(p), ctx);
    }
};

#endif // MPAPP_CAMERA_VIEW_HAS_STD_FORMAT

#endif // MPAPP_INTERNAL_BASIC_CAMERA_VIEW_HPP
