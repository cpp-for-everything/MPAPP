// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::windows_file_picker` — Win32 / COM IFileOpenDialog backend.
// Implements `mpapp::file_picker` using the Windows Shell IFileOpenDialog
// COM interface (Vista+). UTF-8 <-> UTF-16 conversion and all Win32/COM
// headers are confined to the .cpp translation unit. No windows.h, no
// objbase.h, no shobjidl.h in this header.

#ifndef MPAPP_ESSENTIALS_WINDOWS_FILE_PICKER_WINDOWS_HPP
#define MPAPP_ESSENTIALS_WINDOWS_FILE_PICKER_WINDOWS_HPP

#include <optional>
#include <string>
#include <vector>

#include "mpapp/essentials/file_picker.hpp"

namespace mpapp {

// Win32 file-picker backend. Implements `mpapp::file_picker` via the
// Windows Shell IFileOpenDialog COM interface. CoInitializeEx is called
// on each operation (thread-apartment aware, balancing CoUninitialize on
// return). pick() presents a single-selection dialog; pick_multiple()
// sets FOS_ALLOWMULTISELECT. pick_options::file_types are applied as
// COMDLG_FILTERSPEC entries. Returns std::nullopt / empty vector when the
// user cancels (HRESULT_FROM_WIN32(ERROR_CANCELLED)) or on COM failure.
class windows_file_picker final : public file_picker {
public:
    windows_file_picker()  = default;
    ~windows_file_picker() = default;

    windows_file_picker(const windows_file_picker&)            = delete;
    windows_file_picker& operator=(const windows_file_picker&) = delete;
    windows_file_picker(windows_file_picker&&)                 = delete;
    windows_file_picker& operator=(windows_file_picker&&)      = delete;

    // Present the native IFileOpenDialog for a single file selection.
    // Returns std::nullopt if the user cancelled or on any COM error.
    [[nodiscard]] std::optional<file_result>
        pick(const pick_options& options = {}) override;

    // Present the native IFileOpenDialog for multiple file selection
    // (FOS_ALLOWMULTISELECT). Returns an empty vector if the user
    // cancelled or on any COM error.
    [[nodiscard]] std::vector<file_result>
        pick_multiple(const pick_options& options = {}) override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_WINDOWS_FILE_PICKER_WINDOWS_HPP
