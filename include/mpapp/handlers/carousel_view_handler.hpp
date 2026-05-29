// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Cross-platform umbrella header for `carousel_view_handler`.
//
// The umbrella dispatches via `__has_include` to the per-platform header
// matching the build target. Platforms without an implementation yet emit a
// clear `#error` rather than a missing-header diagnostic.
//
// Usage:
//   #include <mpapp/handlers/carousel_view_handler.hpp>
//   mpapp::carousel_view_handler<mpapp::platform::current> handler_{};

#ifndef MPAPP_HANDLERS_CAROUSEL_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_CAROUSEL_VIEW_HANDLER_HPP

#if defined(_WIN32)
#  if __has_include(<mpapp/handlers/windows/carousel_view_handler.hpp>)
#    include <mpapp/handlers/windows/carousel_view_handler.hpp>
#  else
#    error "mpapp::carousel_view_handler<platform::windows> not implemented yet."
#  endif
#elif defined(__ANDROID__)
#  if __has_include(<mpapp/handlers/android/carousel_view_handler.hpp>)
#    include <mpapp/handlers/android/carousel_view_handler.hpp>
#  else
#    error "mpapp::carousel_view_handler<platform::android> not implemented yet."
#  endif
#elif defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE
#    if __has_include(<mpapp/handlers/ios/carousel_view_handler.hpp>)
#      include <mpapp/handlers/ios/carousel_view_handler.hpp>
#    else
#      error "mpapp::carousel_view_handler<platform::ios> not implemented yet — pending Apple host."
#    endif
#  else
#    if __has_include(<mpapp/handlers/macos/carousel_view_handler.hpp>)
#      include <mpapp/handlers/macos/carousel_view_handler.hpp>
#    else
#      error "mpapp::carousel_view_handler<platform::macos> not implemented yet — pending Apple host."
#    endif
#  endif
#elif defined(__linux__)
#  if __has_include(<mpapp/handlers/linux/carousel_view_handler.hpp>)
#    include <mpapp/handlers/linux/carousel_view_handler.hpp>
#  else
#    error "mpapp::carousel_view_handler<platform::linux_> not implemented yet."
#  endif
#else
#  error "MPAPP: unsupported host platform — no `mpapp::carousel_view_handler` available."
#endif

#endif // MPAPP_HANDLERS_CAROUSEL_VIEW_HANDLER_HPP
