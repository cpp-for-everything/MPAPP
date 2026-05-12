# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. Examples — one subdirectory per platform-specific spike.
#
# Each subdirectory is responsible for gating itself on the platforms it
# supports. The current spikes are Windows-only.

if(WIN32)
    add_subdirectory(hot_reload_spike)
    add_subdirectory(windows_button_spike)
endif()
