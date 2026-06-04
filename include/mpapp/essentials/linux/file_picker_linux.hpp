// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::linux_file_picker` — GTK4 GtkFileDialog backend for Linux.
// Implements `mpapp::file_picker` using GtkFileDialog (GTK 4.10+).
// All GTK/GDK/GIO headers are confined to the .cpp translation unit;
// this header exposes only the class declaration.
//
// Synchronous interface strategy:
//   GtkFileDialog is async-only (gtk_file_dialog_open uses GAsyncReadyCallback).
//   To satisfy the synchronous `pick()` / `pick_multiple()` interface, the
//   implementation spins a nested GMainLoop inside the calling thread:
//     1. Open the dialog via gtk_file_dialog_open / gtk_file_dialog_open_multiple.
//     2. In the GAsyncReadyCallback finish the result and quit the nested loop.
//     3. Return the collected result to the caller.
//   This strategy is safe only when called from a thread that owns a GLib main
//   context (typically the GTK main thread). In headless environments
//   (gdk_display_get_default() == null) both methods degrade gracefully:
//   pick() returns std::nullopt, pick_multiple() returns an empty vector.
//
// File-type filtering:
//   pick_options::file_types entries are matched as follows:
//     - Entries beginning with '.' are treated as filename suffixes
//       (gtk_file_filter_add_suffix).
//     - Entries containing '/' are treated as MIME types
//       (gtk_file_filter_add_mime_type).
//     - All other entries are treated as glob patterns
//       (gtk_file_filter_add_pattern).
//   An empty file_types list shows all files (no filter applied).
//
// Thread safety: not thread-safe. Call from the GTK main thread or guard
// externally.

#ifndef MPAPP_ESSENTIALS_LINUX_FILE_PICKER_LINUX_HPP
#define MPAPP_ESSENTIALS_LINUX_FILE_PICKER_LINUX_HPP

#include <optional>
#include <string>
#include <vector>

#include "../../essentials/file_picker.hpp"

namespace mpapp {

// GTK4 GtkFileDialog backend. Implements `mpapp::file_picker` by running a
// nested GMainLoop to bridge the async GTK4 dialog into a synchronous call.
//
// pick() wraps gtk_file_dialog_open in a nested GMainLoop spin.
// pick_multiple() wraps gtk_file_dialog_open_multiple similarly.
//
// Both methods return gracefully (nullopt / empty vector) when:
//   - gdk_display_get_default() returns null (headless/no-display environment).
//   - The user cancels the dialog.
//   - GTK reports an error.
//
// File-type filtering via pick_options::file_types:
//   '.' prefix  → suffix filter  (gtk_file_filter_add_suffix)
//   '/' present → MIME filter    (gtk_file_filter_add_mime_type)
//   otherwise   → glob pattern   (gtk_file_filter_add_pattern)
//
// Thread safety: not thread-safe. Call from the GTK main thread or guard
// externally.
class linux_file_picker final : public file_picker {
public:
    linux_file_picker()  = default;
    ~linux_file_picker() override = default;

    linux_file_picker(const linux_file_picker&)            = delete;
    linux_file_picker& operator=(const linux_file_picker&) = delete;
    linux_file_picker(linux_file_picker&&)                 = delete;
    linux_file_picker& operator=(linux_file_picker&&)      = delete;

    // Present a native GTK4 single-file picker dialog. Blocks the calling
    // thread by spinning a nested GMainLoop until the dialog is dismissed.
    // Returns the selected file_result, or std::nullopt on cancel / error /
    // headless environment.
    [[nodiscard]] std::optional<file_result>
        pick(const pick_options& options = {}) override;

    // Present a native GTK4 multi-file picker dialog. Blocks the calling
    // thread by spinning a nested GMainLoop until the dialog is dismissed.
    // Returns the list of selected file_results (may be empty on cancel /
    // error / headless environment).
    [[nodiscard]] std::vector<file_result>
        pick_multiple(const pick_options& options = {}) override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_LINUX_FILE_PICKER_LINUX_HPP
