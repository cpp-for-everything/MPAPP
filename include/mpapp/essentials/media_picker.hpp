// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::media_picker` — photo and video selection / capture. Counterpart
// to MAUI Essentials `MediaPicker`. Supports picking existing media from the
// gallery and capturing new media via the device camera. Abstract interface
// + an in-memory mock implementation whose canned results and capture-support
// flag are test-settable; each method records its last call arguments.
// Real per-platform backends (Windows FilePicker/Camera, Android
// ActivityResultContracts, iOS UIImagePickerController /
// PHPickerViewController) implement the same interface and are injected via
// the DI container (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_MEDIA_PICKER_HPP
#define MPAPP_ESSENTIALS_MEDIA_PICKER_HPP

#include <optional>
#include <string>

namespace mpapp {

// ---------------------------------------------------------------------------
// Value types
// ---------------------------------------------------------------------------

// A single media file returned by a pick or capture operation. Mirrors MAUI's
// FileResult: full_path is the absolute filesystem path, file_name is the
// base file name, content_type is the MIME type (e.g. "image/jpeg").
struct media_file {
    std::string full_path{};
    std::string file_name{};
    std::string content_type{};

    bool operator==(const media_file&) const = default;
};

// Options passed to pick/capture operations. Mirrors MAUI's
// MediaPickerOptions: title is the user-visible hint shown in the OS picker
// or camera UI (empty means "use the platform default").
struct media_pick_options {
    std::string title{};

    bool operator==(const media_pick_options&) const = default;
};

// ---------------------------------------------------------------------------
// Abstract interface
// ---------------------------------------------------------------------------

class media_picker {
public:
    virtual ~media_picker() = default;

    // Whether the device supports capturing new media (i.e. has a camera).
    // Picking from the gallery is always assumed to be available.
    [[nodiscard]] virtual bool is_capture_supported() const = 0;

    // Open the OS gallery picker and let the user choose a photo.
    // Returns std::nullopt when the user cancels.
    [[nodiscard]] virtual std::optional<media_file>
        pick_photo(const media_pick_options& options = {}) = 0;

    // Open the camera app and let the user capture a new photo.
    // Returns std::nullopt when the user cancels or capture is not supported.
    [[nodiscard]] virtual std::optional<media_file>
        capture_photo(const media_pick_options& options = {}) = 0;

    // Open the OS gallery picker and let the user choose a video.
    // Returns std::nullopt when the user cancels.
    [[nodiscard]] virtual std::optional<media_file>
        pick_video(const media_pick_options& options = {}) = 0;

    // Open the camera app and let the user capture a new video.
    // Returns std::nullopt when the user cancels or capture is not supported.
    [[nodiscard]] virtual std::optional<media_file>
        capture_video(const media_pick_options& options = {}) = 0;
};

// ---------------------------------------------------------------------------
// Mock / in-memory implementation
// ---------------------------------------------------------------------------
// Canned results for all four operations are settable via set_*_result().
// is_capture_supported() is controlled via set_capture_supported().
// When capture is not supported, capture_photo() and capture_video() return
// std::nullopt regardless of the canned result.
// Every method records the options it was last called with; call_count()
// returns the total number of pick/capture calls across all four methods.

class mock_media_picker final : public media_picker {
public:
    explicit mock_media_picker(bool capture_supported = true) noexcept
        : capture_supported_{ capture_supported } {}

    // ---- media_picker interface -------------------------------------------

    [[nodiscard]] bool is_capture_supported() const override {
        return capture_supported_;
    }

    [[nodiscard]] std::optional<media_file>
    pick_photo(const media_pick_options& options = {}) override {
        last_pick_photo_options_ = options;
        ++call_count_;
        return pick_photo_result_;
    }

    [[nodiscard]] std::optional<media_file>
    capture_photo(const media_pick_options& options = {}) override {
        last_capture_photo_options_ = options;
        ++call_count_;
        if (!capture_supported_) {
            return std::nullopt;
        }
        return capture_photo_result_;
    }

    [[nodiscard]] std::optional<media_file>
    pick_video(const media_pick_options& options = {}) override {
        last_pick_video_options_ = options;
        ++call_count_;
        return pick_video_result_;
    }

    [[nodiscard]] std::optional<media_file>
    capture_video(const media_pick_options& options = {}) override {
        last_capture_video_options_ = options;
        ++call_count_;
        if (!capture_supported_) {
            return std::nullopt;
        }
        return capture_video_result_;
    }

    // ---- Test-control helpers: canned results ----------------------------

    // Set the result returned by pick_photo(). Pass std::nullopt to simulate
    // user cancellation.
    void set_pick_photo_result(std::optional<media_file> result) noexcept {
        pick_photo_result_ = std::move(result);
    }

    // Set the result returned by capture_photo() when capture is supported.
    void set_capture_photo_result(std::optional<media_file> result) noexcept {
        capture_photo_result_ = std::move(result);
    }

    // Set the result returned by pick_video().
    void set_pick_video_result(std::optional<media_file> result) noexcept {
        pick_video_result_ = std::move(result);
    }

    // Set the result returned by capture_video() when capture is supported.
    void set_capture_video_result(std::optional<media_file> result) noexcept {
        capture_video_result_ = std::move(result);
    }

    // Change whether the device has a camera.
    void set_capture_supported(bool supported) noexcept {
        capture_supported_ = supported;
    }

    // ---- Test-inspection helpers: last call arguments --------------------

    [[nodiscard]] std::optional<media_pick_options>
    last_pick_photo_options() const noexcept {
        return last_pick_photo_options_;
    }

    [[nodiscard]] std::optional<media_pick_options>
    last_capture_photo_options() const noexcept {
        return last_capture_photo_options_;
    }

    [[nodiscard]] std::optional<media_pick_options>
    last_pick_video_options() const noexcept {
        return last_pick_video_options_;
    }

    [[nodiscard]] std::optional<media_pick_options>
    last_capture_video_options() const noexcept {
        return last_capture_video_options_;
    }

    // Total number of pick/capture method invocations since construction or
    // the last reset().
    [[nodiscard]] int call_count() const noexcept {
        return call_count_;
    }

    // Reset all recorded state and canned results but preserve capture support.
    void reset() noexcept {
        pick_photo_result_           = std::nullopt;
        capture_photo_result_        = std::nullopt;
        pick_video_result_           = std::nullopt;
        capture_video_result_        = std::nullopt;
        last_pick_photo_options_     = std::nullopt;
        last_capture_photo_options_  = std::nullopt;
        last_pick_video_options_     = std::nullopt;
        last_capture_video_options_  = std::nullopt;
        call_count_                  = 0;
    }

private:
    bool capture_supported_ = true;

    std::optional<media_file> pick_photo_result_{};
    std::optional<media_file> capture_photo_result_{};
    std::optional<media_file> pick_video_result_{};
    std::optional<media_file> capture_video_result_{};

    std::optional<media_pick_options> last_pick_photo_options_{};
    std::optional<media_pick_options> last_capture_photo_options_{};
    std::optional<media_pick_options> last_pick_video_options_{};
    std::optional<media_pick_options> last_capture_video_options_{};

    int call_count_ = 0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_MEDIA_PICKER_HPP
