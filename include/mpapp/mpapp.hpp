// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// MPAPP umbrella header.
//
// Canonical entry point for the MPAPP public API. Including this header brings
// the entire user-facing surface of the framework into scope via the mpapp
// namespace. The public API is expressed exclusively through C++ types
// (templates, classes, free functions) -- no preprocessor macros are part of
// the public surface (see ADR-0002, ADR-0009).

#pragma once

#include "button.hpp"
#include "check_box.hpp"
#include "command.hpp"
#include "computed.hpp"
#include "control.hpp"
#include "editor.hpp"
#include "entry.hpp"
#include "label.hpp"
#include "observable.hpp"
#include "platform.hpp"
#include "radio_button.hpp"
#include "signal.hpp"
#include "slider.hpp"
#include "stepper.hpp"
#include "switch_.hpp"

// Layout / primitive component surface (P2 mock — ADR-0008). Mock-only
// users (and the XAML compiler's host-side validation harness) get the
// full layout-group API by including only <mpapp/mpapp.hpp>. Real
// handlers are pulled in explicitly per-platform.
#include "view.hpp"
#include "layout.hpp"
#include "layout_types.hpp"
#include "bindable_layout.hpp"
#include "scroll_view.hpp"
#include "border.hpp"
#include "box_view.hpp"
// `frame` is deprecated; do NOT include it from the umbrella so new
// code is not silently exposed to it. Callers that genuinely need the
// legacy element include <mpapp/frame.hpp> explicitly and accept the
// deprecation diagnostic.

// App-shell surface (T-0011, ADR-0012). Application is the program
// root; window / page are top-level chrome; stack_layout / grid_layout
// are the user-facing layout primitives that replace native
// StackPanel / Grid types in user code. The `mpapp::run<App>`
// entry-point template is here too, but pulls in the native handler
// set for `platform::current` — including <mpapp/run.hpp> brings in
// the real WinUI / GTK4 / AppKit / UIKit / Android handlers and is
// therefore opt-in. Mock-only consumers stick to <mpapp/mpapp.hpp>.
#include "application.hpp"
#include "window.hpp"
#include "page.hpp"
#include "stack_layout.hpp"
#include "grid_layout.hpp"

namespace mpapp {

// Subsystem headers populate this namespace.

} // namespace mpapp
