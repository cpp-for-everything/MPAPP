// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Win32 implementation of `mpapp::windows_file_picker`.
// All COM/Win32 headers are confined to this translation unit.

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objbase.h>
#include <shobjidl.h>

#include <algorithm>
#include <string>
#include <vector>

#include "mpapp/essentials/windows/file_picker_windows.hpp"

namespace {

// ---------------------------------------------------------------------------
// UTF conversion helpers
// ---------------------------------------------------------------------------

// Convert a UTF-8 std::string to a UTF-16 std::wstring.
[[nodiscard]] std::wstring utf8_to_utf16(const std::string& utf8)
{
    if (utf8.empty()) {
        return {};
    }
    const int required = ::MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS,
        utf8.data(), static_cast<int>(utf8.size()),
        nullptr, 0);
    if (required <= 0) {
        return {};
    }
    std::wstring utf16(static_cast<std::size_t>(required), L'\0');
    ::MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS,
        utf8.data(), static_cast<int>(utf8.size()),
        utf16.data(), required);
    return utf16;
}

// Convert a null-terminated UTF-16 wide string to a UTF-8 std::string.
[[nodiscard]] std::string utf16_to_utf8(const wchar_t* utf16)
{
    if (!utf16 || utf16[0] == L'\0') {
        return {};
    }
    const int len = static_cast<int>(::wcslen(utf16));
    const int required = ::WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS,
        utf16, len,
        nullptr, 0,
        nullptr, nullptr);
    if (required <= 0) {
        return {};
    }
    std::string utf8(static_cast<std::size_t>(required), '\0');
    ::WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS,
        utf16, len,
        utf8.data(), required,
        nullptr, nullptr);
    return utf8;
}

// ---------------------------------------------------------------------------
// RAII COM initialiser (per-call, STA)
// ---------------------------------------------------------------------------

// Calls CoInitializeEx on construction; CoUninitialize on destruction.
// ok() is true if COM was initialised (or was already initialised) by this
// call — S_OK or S_FALSE both count as usable.
class scoped_coinit {
public:
    explicit scoped_coinit() noexcept
        : hr_{ ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED) }
    {}

    ~scoped_coinit() noexcept {
        if (SUCCEEDED(hr_)) {
            ::CoUninitialize();
        }
    }

    scoped_coinit(const scoped_coinit&)            = delete;
    scoped_coinit& operator=(const scoped_coinit&) = delete;
    scoped_coinit(scoped_coinit&&)                 = delete;
    scoped_coinit& operator=(scoped_coinit&&)      = delete;

    // Returns true when COM is available for use on this thread.
    [[nodiscard]] bool ok() const noexcept { return SUCCEEDED(hr_); }

private:
    HRESULT hr_;
};

// ---------------------------------------------------------------------------
// RAII COM pointer wrapper (minimal, avoids ATL/WRL dependency)
// ---------------------------------------------------------------------------

template <typename T>
class com_ptr {
public:
    com_ptr() noexcept : ptr_{ nullptr } {}

    ~com_ptr() noexcept {
        reset();
    }

    com_ptr(const com_ptr&)            = delete;
    com_ptr& operator=(const com_ptr&) = delete;

    com_ptr(com_ptr&& other) noexcept : ptr_{ other.ptr_ } {
        other.ptr_ = nullptr;
    }
    com_ptr& operator=(com_ptr&& other) noexcept {
        if (this != &other) {
            reset();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    void reset() noexcept {
        if (ptr_) {
            ptr_->Release();
            ptr_ = nullptr;
        }
    }

    [[nodiscard]] T* get() const noexcept { return ptr_; }
    [[nodiscard]] T* operator->() const noexcept { return ptr_; }
    [[nodiscard]] T** address_of() noexcept { return &ptr_; }

    [[nodiscard]] explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

private:
    T* ptr_;
};

// ---------------------------------------------------------------------------
// COMDLG_FILTERSPEC builder
// ---------------------------------------------------------------------------

// Builds a vector of COMDLG_FILTERSPEC from a list of file-type strings.
// Each string is treated as a file extension pattern (e.g. ".txt", "*.png").
// Strings that already start with '*' are used verbatim; otherwise ";" are
// left unchanged. A single "All Files (*.*)" entry covers everything when
// the list is empty.
//
// Storage for the wide strings must outlive the returned specs; we return
// both together.
struct filter_storage {
    // Wide string storage — each entry is "Display Name\0*.ext\0"
    std::vector<std::wstring>   display_names;
    std::vector<std::wstring>   patterns;
    std::vector<COMDLG_FILTERSPEC> specs;
};

[[nodiscard]] filter_storage build_filters(
    const std::vector<std::string>& file_types)
{
    filter_storage fs;

    if (file_types.empty()) {
        fs.display_names.push_back(L"All Files");
        fs.patterns.push_back(L"*.*");
    } else {
        for (const auto& ft : file_types) {
            std::wstring pattern = utf8_to_utf16(ft);
            // Normalise: strip leading dot; build *.ext
            if (!pattern.empty() && pattern[0] == L'.') {
                pattern = L"*" + pattern;
            } else if (!pattern.empty() && pattern[0] != L'*') {
                pattern = L"*." + pattern;
            }
            std::wstring display = pattern; // use the pattern itself as display
            fs.display_names.push_back(std::move(display));
            fs.patterns.push_back(std::move(pattern));
        }
    }

    fs.specs.resize(fs.display_names.size());
    for (std::size_t i = 0; i < fs.display_names.size(); ++i) {
        fs.specs[i].pszName = fs.display_names[i].c_str();
        fs.specs[i].pszSpec = fs.patterns[i].c_str();
    }

    return fs;
}

// ---------------------------------------------------------------------------
// Single IShellItem -> file_result
// ---------------------------------------------------------------------------

[[nodiscard]] std::optional<mpapp::file_result>
shell_item_to_result(IShellItem* item)
{
    if (!item) {
        return std::nullopt;
    }

    PWSTR display_name = nullptr;
    HRESULT hr = item->GetDisplayName(SIGDN_FILESYSPATH, &display_name);
    if (FAILED(hr) || !display_name) {
        return std::nullopt;
    }

    // RAII for the COM-allocated string.
    struct pwstr_guard {
        PWSTR ptr;
        ~pwstr_guard() noexcept { if (ptr) ::CoTaskMemFree(ptr); }
    } guard{ display_name };

    std::string full_path = utf16_to_utf8(display_name);
    if (full_path.empty()) {
        return std::nullopt;
    }

    // Derive the file_name as the last path component.
    std::string file_name;
    const auto slash_pos = full_path.find_last_of("/\\");
    if (slash_pos != std::string::npos) {
        file_name = full_path.substr(slash_pos + 1u);
    } else {
        file_name = full_path;
    }

    return mpapp::file_result{
        std::move(full_path),
        std::move(file_name),
        {} // content_type not available from IFileOpenDialog
    };
}

// ---------------------------------------------------------------------------
// Core helper: run IFileOpenDialog with the given flags
// ---------------------------------------------------------------------------

[[nodiscard]] std::vector<mpapp::file_result>
run_file_open_dialog(
    const mpapp::pick_options& options,
    FILEOPENDIALOGOPTIONS extra_flags)
{
    scoped_coinit coinit;
    if (!coinit.ok()) {
        return {};
    }

    com_ptr<IFileOpenDialog> dialog;
    HRESULT hr = ::CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IFileOpenDialog,
        reinterpret_cast<void**>(dialog.address_of()));
    if (FAILED(hr)) {
        return {};
    }

    // Set dialog title if provided.
    if (!options.title.empty()) {
        std::wstring wtitle = utf8_to_utf16(options.title);
        dialog->SetTitle(wtitle.c_str());
    }

    // Apply file-type filters.
    filter_storage fs = build_filters(options.file_types);
    if (!fs.specs.empty()) {
        dialog->SetFileTypes(
            static_cast<UINT>(fs.specs.size()),
            fs.specs.data());
    }

    // Set requested flags (preserve existing options).
    {
        FILEOPENDIALOGOPTIONS current_flags = 0;
        hr = dialog->GetOptions(&current_flags);
        if (SUCCEEDED(hr)) {
            dialog->SetOptions(current_flags | extra_flags);
        }
    }

    // Show the dialog. ERROR_CANCELLED means user dismissed — not an error.
    hr = dialog->Show(nullptr);
    if (FAILED(hr)) {
        return {};
    }

    // Retrieve results.
    std::vector<mpapp::file_result> results;

    if (extra_flags & FOS_ALLOWMULTISELECT) {
        com_ptr<IShellItemArray> item_array;
        hr = dialog->GetResults(item_array.address_of());
        if (FAILED(hr) || !item_array) {
            return {};
        }

        DWORD count = 0;
        hr = item_array->GetCount(&count);
        if (FAILED(hr)) {
            return {};
        }

        results.reserve(static_cast<std::size_t>(count));
        for (DWORD i = 0; i < count; ++i) {
            com_ptr<IShellItem> item;
            hr = item_array->GetItemAt(i, item.address_of());
            if (FAILED(hr) || !item) {
                continue;
            }
            auto result = shell_item_to_result(item.get());
            if (result) {
                results.push_back(std::move(*result));
            }
        }
    } else {
        com_ptr<IShellItem> item;
        hr = dialog->GetResult(item.address_of());
        if (FAILED(hr) || !item) {
            return {};
        }
        auto result = shell_item_to_result(item.get());
        if (result) {
            results.push_back(std::move(*result));
        }
    }

    return results;
}

} // anonymous namespace

namespace mpapp {

std::optional<file_result>
windows_file_picker::pick(const pick_options& options)
{
    auto results = run_file_open_dialog(options, 0);
    if (results.empty()) {
        return std::nullopt;
    }
    return std::move(results.front());
}

std::vector<file_result>
windows_file_picker::pick_multiple(const pick_options& options)
{
    return run_file_open_dialog(options, FOS_ALLOWMULTISELECT);
}

} // namespace mpapp

#endif // defined(_WIN32)
