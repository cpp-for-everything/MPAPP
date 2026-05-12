// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Hot Reload.md and
// vault/50_Tasks/T-0010-hot-reload-spike/.
//
// Hot-reload public surface (Windows desktop spike).
//
// This header exposes:
//
//   * mpapp::Hot<T>         — empty tag base used by user code to opt a class
//                             into the hot-reload runtime (per ADR-0009, the
//                             public surface uses template wrapper types, not
//                             macros).
//   * mpapp::hot_reload::runtime — the runtime that watches a .cpp source,
//                             rebuilds it as a shared library, and swaps the
//                             loaded image in-process.
//
// The runtime is intentionally minimal for the T-0010 spike: it polls mtime
// on the watched source, invokes a C++ compiler (clang++ by default) to
// produce a shared library beside the host, and reloads the resulting image.
// State preservation across the swap is out of scope for this spike — the
// pattern is demonstrated by re-acquiring the `compute(int)` function pointer
// after the swap and observing new behavior.

#ifndef MPAPP_HOT_RELOAD_HPP
#define MPAPP_HOT_RELOAD_HPP

#include <filesystem>
#include <string>

namespace mpapp {

// Empty tag base. Inheriting from Hot<T> opts a user type into the hot-reload
// runtime's class registry. The mechanism for state preservation is layered
// on top of this in later tasks; for the T-0010 spike, presence of the base
// class is the entire contract.
template <class T>
struct Hot {
protected:
    Hot()  = default;
    ~Hot() = default;
};

namespace hot_reload {

// A handle to the OS shared-library image. Kept as void* so the public header
// stays free of <windows.h>. The implementation reinterpret_casts to HMODULE
// in src/hot_reload/windows.cpp.
using library_handle = void*;

class runtime {
public:
    // watched_source must point to a .cpp file the runtime is allowed to
    // recompile. The runtime derives the library output path from it
    // (next to the host executable) and performs an initial build + load
    // during construction.
    explicit runtime(std::filesystem::path watched_source);

    ~runtime();

    runtime(const runtime&)            = delete;
    runtime& operator=(const runtime&) = delete;
    runtime(runtime&&)                 = delete;
    runtime& operator=(runtime&&)      = delete;

    // Returns the current `compute(int)` function pointer. Stable until the
    // next successful poll() that triggers a swap; callers must re-acquire
    // after a reload.
    auto compute() const -> int (*)(int);

    // Polls the watched source's mtime. If it has advanced since the last
    // successful load, the runtime rebuilds the shared library and swaps it
    // in. Returns true on a successful swap; false otherwise (no change, or
    // a rebuild/load error — see last_error()).
    bool poll();

    // Compiler invocation used by the runtime. Defaults to "clang++".
    // Overridable for environments where clang++ is not on PATH.
    void set_compiler(std::string compiler) { compiler_ = std::move(compiler); }

    // Diagnostic string set when a poll() rebuild or load fails. Empty on
    // success.
    const std::string& last_error() const noexcept { return last_error_; }

private:
    bool rebuild_();
    bool load_();
    void unload_() noexcept;

    std::filesystem::path           source_;
    std::filesystem::path           library_;
    std::filesystem::file_time_type last_mtime_{};
    library_handle                  dll_handle_  = nullptr;
    int (*compute_ptr_)(int)                     = nullptr;
    std::string                     compiler_    = "clang++";
    std::string                     last_error_;
};

} // namespace hot_reload
} // namespace mpapp

#endif // MPAPP_HOT_RELOAD_HPP
