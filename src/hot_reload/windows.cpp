// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/50_Tasks/T-0010-hot-reload-spike/.
//
// Windows implementation of mpapp::hot_reload::runtime.
//
// The runtime watches a single .cpp source's mtime, invokes the configured
// compiler to produce a sibling .dll next to the host executable, and uses
// LoadLibraryExW / FreeLibrary to swap the image in-process. The exported
// entry point is a C function named `compute` taking and returning int.

#include <mpapp/hot_reload.hpp>

#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>

#if !defined(_WIN32)
#  error "src/hot_reload/windows.cpp is Windows-only."
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace mpapp::hot_reload {

namespace {

// Quote a path for cmd.exe so spaces don't split arguments.
std::string quote_(const std::filesystem::path& p) {
    std::string s = p.string();
    return std::string{"\""} + s + "\"";
}

} // namespace

runtime::runtime(std::filesystem::path watched_source)
    : source_(std::move(watched_source)) {
    // Derive the library path: next to the source, with .dll extension.
    library_ = source_;
    library_.replace_extension(".dll");

    if (!rebuild_()) {
        // last_error_ populated by rebuild_().
        return;
    }
    if (!load_()) {
        return;
    }

    std::error_code ec;
    last_mtime_ = std::filesystem::last_write_time(source_, ec);
}

runtime::~runtime() {
    unload_();
}

auto runtime::compute() const -> int (*)(int) {
    return compute_ptr_;
}

bool runtime::poll() {
    last_error_.clear();
    std::error_code ec;
    auto mtime = std::filesystem::last_write_time(source_, ec);
    if (ec) {
        last_error_ = "stat failed: " + ec.message();
        return false;
    }
    if (mtime <= last_mtime_) {
        return false;
    }

    // Source changed — rebuild + swap.
    unload_();
    if (!rebuild_() || !load_()) {
        return false;
    }
    last_mtime_ = mtime;
    return true;
}

bool runtime::rebuild_() {
    // clang++ -std=c++23 -shared -o user_code.dll user_code.cpp
    // Quoting handles spaces in paths.
    std::string cmd;
    cmd.reserve(256);
    cmd += compiler_;
    cmd += " -std=c++23 -shared -o ";
    cmd += quote_(library_);
    cmd += ' ';
    cmd += quote_(source_);

    int rc = std::system(cmd.c_str());
    if (rc != 0) {
        last_error_ = "rebuild failed (exit " + std::to_string(rc) + "): " + cmd;
        return false;
    }
    return true;
}

bool runtime::load_() {
    // LoadLibraryExW with LOAD_WITH_ALTERED_SEARCH_PATH so the dll's own
    // directory wins for any sibling deps it might depend on.
    const std::wstring wpath = library_.wstring();
    HMODULE h = ::LoadLibraryExW(wpath.c_str(), nullptr,
                                 LOAD_WITH_ALTERED_SEARCH_PATH);
    if (h == nullptr) {
        DWORD err = ::GetLastError();
        last_error_ = "LoadLibraryExW failed: error " + std::to_string(err);
        return false;
    }

    auto* sym = reinterpret_cast<int (*)(int)>(
        reinterpret_cast<void*>(::GetProcAddress(h, "compute")));
    if (sym == nullptr) {
        DWORD err = ::GetLastError();
        ::FreeLibrary(h);
        last_error_ = "GetProcAddress(compute) failed: error "
                      + std::to_string(err);
        return false;
    }

    dll_handle_  = reinterpret_cast<library_handle>(h);
    compute_ptr_ = sym;
    return true;
}

void runtime::unload_() noexcept {
    if (dll_handle_ != nullptr) {
        ::FreeLibrary(reinterpret_cast<HMODULE>(dll_handle_));
        dll_handle_  = nullptr;
        compute_ptr_ = nullptr;
    }
}

} // namespace mpapp::hot_reload
