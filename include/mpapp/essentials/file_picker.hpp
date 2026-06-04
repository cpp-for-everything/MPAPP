// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::file_picker` — native file selection dialog. Counterpart to
// MAUI Essentials `FilePicker`. Allows the user to pick one or multiple
// files from the native OS file browser. Abstract interface + an
// in-memory mock implementation whose canned results are settable so
// tests can drive and verify file-picker interactions. Real per-platform
// backends (Windows StoragePicker, GTK/portal on Linux, Android
// Storage Access Framework, iOS UIDocumentPickerViewController)
// implement the same interface and are injected via the DI container
// (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_FILE_PICKER_HPP
#define MPAPP_ESSENTIALS_FILE_PICKER_HPP

#include <optional>
#include <string>
#include <vector>

namespace mpapp {

// ---------------------------------------------------------------------------
// Result value type
// ---------------------------------------------------------------------------

// Mirrors MAUI's FileResult: full_path is the absolute path to the
// selected file, file_name is the basename, content_type is a MIME type
// hint (may be empty if the platform cannot determine it).
struct file_result {
    std::string full_path{};
    std::string file_name{};
    std::string content_type{};

    bool operator==(const file_result&) const = default;
};

// ---------------------------------------------------------------------------
// Options value type
// ---------------------------------------------------------------------------

// Mirrors MAUI's PickOptions: title is shown in the dialog title bar,
// file_types restricts which extensions / MIME types are visible
// (empty means all files are shown).
struct pick_options {
    std::string              title{};
    std::vector<std::string> file_types{};

    bool operator==(const pick_options&) const = default;
};

// ---------------------------------------------------------------------------
// Abstract interface
// ---------------------------------------------------------------------------

class file_picker {
public:
    virtual ~file_picker() = default;

    // Present the OS file picker for a single selection. Returns the
    // selected file, or std::nullopt if the user cancelled or the
    // operation is not supported on this platform.
    [[nodiscard]] virtual std::optional<file_result>
        pick(const pick_options& options = {}) = 0;

    // Present the OS file picker for multiple selections. Returns the
    // list of selected files (may be empty if the user cancelled or if
    // the operation is not supported on this platform).
    [[nodiscard]] virtual std::vector<file_result>
        pick_multiple(const pick_options& options = {}) = 0;
};

// ---------------------------------------------------------------------------
// Mock / in-memory implementation
// ---------------------------------------------------------------------------
// Canned results are settable; pick() / pick_multiple() return the
// pre-loaded results and record the options used for each call so tests
// can inspect which options were forwarded by the caller.

class mock_file_picker final : public file_picker {
public:
    mock_file_picker() = default;

    // ---- Canned result setters --------------------------------------------

    // Set the result returned by the next (and subsequent) pick() calls.
    // Pass std::nullopt to simulate a cancelled / unsupported pick.
    void set_pick_result(std::optional<file_result> result) {
        canned_single_ = std::move(result);
    }

    // Set the results returned by the next (and subsequent) pick_multiple()
    // calls. Pass an empty vector to simulate cancellation.
    void set_pick_multiple_results(std::vector<file_result> results) {
        canned_multiple_ = std::move(results);
    }

    // ---- Abstract interface implementation --------------------------------

    [[nodiscard]] std::optional<file_result>
    pick(const pick_options& options = {}) override {
        last_pick_options_ = options;
        ++pick_call_count_;
        return canned_single_;
    }

    [[nodiscard]] std::vector<file_result>
    pick_multiple(const pick_options& options = {}) override {
        last_pick_multiple_options_ = options;
        ++pick_multiple_call_count_;
        return canned_multiple_;
    }

    // ---- Inspection helpers -----------------------------------------------

    // Returns the options passed to the most recent pick() call, or
    // std::nullopt if pick() has never been called.
    [[nodiscard]] std::optional<pick_options> last_pick_options() const {
        return last_pick_options_;
    }

    // Returns the options passed to the most recent pick_multiple() call,
    // or std::nullopt if pick_multiple() has never been called.
    [[nodiscard]] std::optional<pick_options> last_pick_multiple_options() const {
        return last_pick_multiple_options_;
    }

    // Returns the number of times pick() has been called.
    [[nodiscard]] int pick_call_count() const noexcept {
        return pick_call_count_;
    }

    // Returns the number of times pick_multiple() has been called.
    [[nodiscard]] int pick_multiple_call_count() const noexcept {
        return pick_multiple_call_count_;
    }

    // Reset all recorded state and canned results.
    void reset() noexcept {
        canned_single_             = std::nullopt;
        canned_multiple_           = {};
        last_pick_options_         = std::nullopt;
        last_pick_multiple_options_ = std::nullopt;
        pick_call_count_           = 0;
        pick_multiple_call_count_  = 0;
    }

private:
    std::optional<file_result>  canned_single_{};
    std::vector<file_result>    canned_multiple_{};

    std::optional<pick_options> last_pick_options_{};
    std::optional<pick_options> last_pick_multiple_options_{};

    int pick_call_count_          = 0;
    int pick_multiple_call_count_ = 0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_FILE_PICKER_HPP
