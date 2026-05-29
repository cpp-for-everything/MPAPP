// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Compatibility shim for `<stop_token>` on platforms where
// the standard library doesn't ship it yet (Android NDK 26 libc++ is the
// notable case). On hosts that have it, alias to the std types; otherwise
// provide a minimal implementation covering the surface mpapp uses
// (default-construct, get_token, request_stop, stop_requested).
//
// The mpapp namespace exposes `stop_source` + `stop_token` regardless;
// callers should use those, not `std::stop_token` directly.

#ifndef MPAPP_DETAIL_STOP_TOKEN_COMPAT_HPP
#define MPAPP_DETAIL_STOP_TOKEN_COMPAT_HPP

// Detect a *usable* std::stop_token, not merely the header's presence.
// Android NDK 27 libc++ ships <stop_token> but gates the types out (no
// jthread support), so `__has_include` alone is a false positive. The
// `__cpp_lib_jthread` feature-test macro is defined only when stop_token /
// stop_source / stop_callback are actually available, so gate on it.
#if __has_include(<version>)
#  include <version>
#endif
#if __has_include(<stop_token>) && defined(__cpp_lib_jthread)
#  include <stop_token>
#  define MPAPP_HAS_STD_STOP_TOKEN 1
#endif

#include <atomic>
#include <memory>

namespace mpapp {

#if defined(MPAPP_HAS_STD_STOP_TOKEN)

using stop_source = std::stop_source;
using stop_token  = std::stop_token;

#else

// Fallback when libc++ doesn't ship <stop_token>. Minimal version covering
// the exact surface executor.hpp uses; no stop_callback, no condition_var
// integration, no atomic-wait. Sufficient for cooperative cancellation
// inside coroutines.

class stop_token {
public:
    stop_token() = default;
    explicit stop_token(std::shared_ptr<std::atomic<bool>> flag) noexcept
        : flag_(std::move(flag)) {}

    [[nodiscard]] bool stop_requested() const noexcept {
        return flag_ && flag_->load(std::memory_order_acquire);
    }
    [[nodiscard]] bool stop_possible() const noexcept {
        return static_cast<bool>(flag_);
    }

    bool operator==(const stop_token& o) const noexcept { return flag_ == o.flag_; }
    bool operator!=(const stop_token& o) const noexcept { return !(*this == o); }

private:
    std::shared_ptr<std::atomic<bool>> flag_{};
};

class stop_source {
public:
    stop_source() : flag_(std::make_shared<std::atomic<bool>>(false)) {}

    bool request_stop() noexcept {
        if (!flag_) return false;
        bool prev = flag_->exchange(true, std::memory_order_release);
        return !prev; // true if this call did the request
    }
    [[nodiscard]] bool stop_requested() const noexcept {
        return flag_ && flag_->load(std::memory_order_acquire);
    }
    [[nodiscard]] bool stop_possible() const noexcept {
        return static_cast<bool>(flag_);
    }
    [[nodiscard]] stop_token get_token() const noexcept {
        return stop_token{flag_};
    }

private:
    std::shared_ptr<std::atomic<bool>> flag_{};
};

#endif // MPAPP_HAS_STD_STOP_TOKEN

} // namespace mpapp

#endif // MPAPP_DETAIL_STOP_TOKEN_COMPAT_HPP
