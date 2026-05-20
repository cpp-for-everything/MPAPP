// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Per-ADR-0013 data-driven widget dispatch — GTK4.

#ifndef MPAPP_HANDLERS_LINUX_WIDGET_DISPATCH_HPP
#define MPAPP_HANDLERS_LINUX_WIDGET_DISPATCH_HPP

#if defined(__linux__) && !defined(__ANDROID__)

extern "C" {
    // Forward-declare GtkWidget* without dragging gtk/gtk.h into the header.
    typedef struct _GtkWidget GtkWidget;
}

namespace mpapp { class view; }

namespace mpapp::detail::linux_dispatch {

using dispatcher_fn = GtkWidget* (*)(::mpapp::view*);

void register_dispatcher(dispatcher_fn fn);

GtkWidget* dispatch(::mpapp::view* v);

} // namespace mpapp::detail::linux_dispatch

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_WIDGET_DISPATCH_HPP
