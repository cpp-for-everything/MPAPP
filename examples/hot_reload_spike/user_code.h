// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0010 hot-reload spike — user-side header.
//
// This declares the single symbol the host program looks up in the
// hot-reloaded shared library. The `extern "C"` linkage keeps the mangled
// name stable across rebuilds and avoids ABI surprises in the spike.

#ifndef MPAPP_HOT_RELOAD_SPIKE_USER_CODE_H
#define MPAPP_HOT_RELOAD_SPIKE_USER_CODE_H

#if defined(_WIN32)
#  define USER_CODE_EXPORT __declspec(dllexport)
#else
#  define USER_CODE_EXPORT
#endif

extern "C" USER_CODE_EXPORT int compute(int x);

#endif // MPAPP_HOT_RELOAD_SPIKE_USER_CODE_H
