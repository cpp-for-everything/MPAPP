// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 wiring for the RFC-0003 gesture-recognizer family.
//
// Per-component Linux handlers (button_handler, label_handler, ...)
// call `attach_gestures(widget, view)` after creating their native
// `GtkWidget*` so every recognizer in `view.gesture_recognizers` gets
// the appropriate GtkGesture* controller installed on the widget.
//
// Lifetime: the GTK controller is parented to the widget via
// `gtk_widget_add_controller(widget, controller)` — the widget owns it
// and destroys it during widget teardown. Recognizers live on the
// `view::gesture_recognizers` vector, which destroys AFTER the platform
// handler (and therefore after the native widget), so the controller's
// callback can hold a raw pointer to the recognizer safely.

#ifndef MPAPP_HANDLERS_LINUX_GESTURE_ATTACH_HPP
#define MPAPP_HANDLERS_LINUX_GESTURE_ATTACH_HPP

#include "../../platform.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp {
class view;
} // namespace mpapp

namespace mpapp::internal::linux_gestures {

// Walk `v.gesture_recognizers` and install the matching GtkGesture* /
// GtkEventController* on `widget` for each one. Idempotent only in the
// sense that re-attaching duplicates the controllers — callers should
// invoke once per widget setup.
void attach(GtkWidget* widget, view& v);

} // namespace mpapp::internal::linux_gestures

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_GESTURE_ATTACH_HPP
