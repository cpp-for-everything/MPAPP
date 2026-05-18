// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// Platform-conditional include of the *real* native handler set. User
// code that wants to run on `platform::current` includes this header
// instead of naming the per-platform path explicitly, so the same .cpp
// compiles unmodified on every supported host.
//
// Mock handlers are explicitly *not* pulled in here — they live behind
// `mpapp/handlers/mock/...` and are only used by tests + the
// XAML-compiler validation harness.
//
// Rule 1 (no public-API macros) covers user-facing surface only; the
// platform conditional below is an internal preprocessor guard, which
// ADR-0002 explicitly exempts.

#ifndef MPAPP_NATIVE_HANDLERS_HPP
#define MPAPP_NATIVE_HANDLERS_HPP

#include "platform.hpp"

#if defined(_WIN32)
#  include "handlers/windows/application_handler.hpp"
#  include "handlers/windows/window_handler.hpp"
#  include "handlers/windows/stack_layout_handler.hpp"
// `button_handler` / `label_handler` / `page_handler` / `grid_layout_handler`
// are pulled in by user code that needs them. The Application / Window /
// StackLayout trio is what `mpapp::run<App>` needs to instantiate.
#elif defined(__ANDROID__)
// Android handlers — TODO(T-0011 follow-up): land alongside the JNI
// codegen work in T-0004. For now this is a hard error so misconfigured
// builds fail loudly instead of silently dropping the entry point.
#  error "MPAPP: Android handlers not yet implemented (tracked in T-0011 follow-up)"
#elif defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE
#    error "MPAPP: iOS handlers not yet implemented (tracked in T-0011 follow-up)"
#  else
#    error "MPAPP: macOS handlers not yet implemented (tracked in T-0011 follow-up)"
#  endif
#elif defined(__linux__)
#  error "MPAPP: Linux/GTK4 handlers not yet implemented (tracked in T-0011 follow-up + T-0007 unblocking)"
#else
#  error "MPAPP: unsupported host platform — no native handler set available."
#endif

#endif // MPAPP_NATIVE_HANDLERS_HPP
