# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. Examples — one subdirectory per platform-specific spike.
#
# Each subdirectory is responsible for gating itself on the platforms it
# supports. Windows hosts the WinUI 3 spikes; Linux (incl. WSLg from a
# Windows host) hosts the GTK4 hello-window smoke target.

if(WIN32)
    add_subdirectory(hot_reload_spike)
    add_subdirectory(windows_button_spike)
    add_subdirectory(windows_nav_spike)
endif()

if(UNIX AND NOT APPLE)
    add_subdirectory(gtk4_hello)
    add_subdirectory(gtk4_tableview_demo)
endif()
