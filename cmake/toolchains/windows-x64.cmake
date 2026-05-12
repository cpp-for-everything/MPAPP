# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP.
#
# windows-x64 host-native toolchain stub.
#
# This file exists so the `mpapp build --target windows-x64` lookup
# succeeds on a fresh checkout. It does not currently set the compiler
# or any cross-compile variables — windows-x64 is also the assumed host
# in the test matrix, so the default toolchain still applies. As the
# build system grows, this file gains the explicit MSVC selection and
# arch flags it currently inherits implicitly.

set(MPAPP_TARGET_TRIPLE "windows-x64" CACHE STRING "MPAPP target triple")
