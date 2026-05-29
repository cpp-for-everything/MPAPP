// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/50_Tasks/T-0010-hot-reload-spike/.
//
// Linux implementation of mpapp::hot_reload::runtime — the POSIX `dlopen`
// counterpart of src/hot_reload/windows.cpp. The runtime watches a single
// .cpp source's mtime, invokes the configured compiler to produce a sibling
// .so, and uses dlopen / dlclose to swap the image in-process. The exported
// entry point is a C function named `compute` taking and returning int.
//
// "Hot-reload for quick iterations during development" (the project goal):
// same minimal mechanism on every desktop platform, one file per OS, no
// ifdefs in the public surface (mpapp/hot_reload.hpp stays platform-free).

#include <mpapp/hot_reload.hpp>

#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>

#if !(defined(__linux__) && !defined(__ANDROID__))
#  error "src/hot_reload/linux.cpp is Linux-desktop-only."
#endif

#include <dlfcn.h>

namespace mpapp::hot_reload {

namespace {

// Quote a path for /bin/sh so spaces don't split arguments.
std::string quote_(const std::filesystem::path& p) {
    return std::string{"\""} + p.string() + "\"";
}

} // namespace

runtime::runtime(std::filesystem::path watched_source)
    : source_(std::move(watched_source)) {
    // Derive the library path: next to the source, with .so extension.
    library_ = source_;
    library_.replace_extension(".so");

    if (!rebuild_()) {
        return;   // last_error_ populated by rebuild_().
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
    // <compiler> -std=c++23 -shared -fPIC -o user_code.so user_code.cpp
    std::string cmd;
    cmd.reserve(256);
    cmd += compiler_;
    cmd += " -std=c++23 -shared -fPIC -o ";
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
    // Absolute path so dlopen loads this image directly rather than
    // searching the standard library paths. RTLD_NOW resolves all symbols
    // up front (so a bad swap fails here, not at first call); RTLD_LOCAL
    // keeps the image's symbols out of the global namespace so successive
    // reloads don't clash.
    std::error_code ec;
    std::filesystem::path abs = std::filesystem::absolute(library_, ec);
    const std::string path = (ec ? library_ : abs).string();

    void* h = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (h == nullptr) {
        const char* e = ::dlerror();
        last_error_ = std::string{"dlopen failed: "} + (e ? e : "(unknown)");
        return false;
    }

    ::dlerror();   // clear any stale error
    auto* sym = reinterpret_cast<int (*)(int)>(
        reinterpret_cast<std::uintptr_t>(::dlsym(h, "compute")));
    if (sym == nullptr) {
        const char* e = ::dlerror();
        ::dlclose(h);
        last_error_ = std::string{"dlsym(compute) failed: "} + (e ? e : "(unknown)");
        return false;
    }

    dll_handle_  = reinterpret_cast<library_handle>(h);
    compute_ptr_ = sym;
    return true;
}

void runtime::unload_() noexcept {
    if (dll_handle_ != nullptr) {
        ::dlclose(dll_handle_);
        dll_handle_  = nullptr;
        compute_ptr_ = nullptr;
    }
}

} // namespace mpapp::hot_reload
