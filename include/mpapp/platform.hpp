// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Platform Interop.md
//
// Platform tag types. These are empty struct types used as template
// arguments to specialise handlers per platform — e.g.
// `button_handler<platform::windows>`. They are NOT a runtime enum: the
// active platform is selected at compile time and `platform::current`
// resolves to the host tag, allowing handler code to be selected with
// `if constexpr` or partial template specialisation.
//
// No public-API preprocessor macros are introduced. Internal build-time
// guards (`#if defined(_WIN32)` etc.) are exempt from Rule 1 — they are
// implementation details, not user-facing surface.

#ifndef MPAPP_PLATFORM_HPP
#define MPAPP_PLATFORM_HPP

namespace mpapp::platform {

struct windows {};
struct android {};
struct linux_  {}; // 'linux' is a predefined macro in some toolchains.
struct macos   {};
struct ios     {};

// Mock tag — used by the P2 mock surface (ADR-0008). Selecting it via a
// handler template specialisation gives a host-independent recording
// handler that runs in CI on every platform, regardless of which native
// SDK is installed. Never `platform::current`.
struct mock    {};

#if defined(_WIN32)
using current = windows;
#elif defined(__ANDROID__)
using current = android;
#elif defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE
using current = ios;
#  else
using current = macos;
#  endif
#elif defined(__linux__)
using current = linux_;
#else
#  error "MPAPP: unsupported host platform — no platform::current available."
#endif

} // namespace mpapp::platform

#endif // MPAPP_PLATFORM_HPP
