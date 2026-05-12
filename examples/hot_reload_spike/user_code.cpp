// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0010 hot-reload spike — the "user code".
//
// This translation unit is built into user_code.dll by the host's runtime.
// Edit the body of `compute` while host.exe is running and the runtime will
// detect the mtime change, rebuild this file via clang++ -shared, and swap
// the loaded image in-process.

#include "user_code.h"

extern "C" USER_CODE_EXPORT int compute(int x) {
    // Initial behavior — doubles its argument. Try changing this to
    // `return x * 10;` (or anything else) while the host runs.
    return x * 2;
}
