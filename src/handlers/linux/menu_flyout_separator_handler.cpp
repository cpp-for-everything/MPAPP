// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 menu_flyout_separator handler implementation.

#include "mpapp/handlers/linux/menu_flyout_separator_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

#include "mpapp/menu_flyout_separator.hpp"

namespace mpapp {

menu_flyout_separator_handler<platform::linux_>::menu_flyout_separator_handler() {
    native_ = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
}

menu_flyout_separator_handler<platform::linux_>::~menu_flyout_separator_handler() = default;

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --

namespace {

GtkWidget* dispatch_menu_flyout_separator(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::menu_flyout_separator*>(v); s && s->has_handler()) {
        return GTK_WIDGET(static_cast<GtkWidget*>(s->handler().native()));
    }
    return nullptr;
}

struct registrar_mfs {
    registrar_mfs() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_menu_flyout_separator); }
};

[[maybe_unused]] registrar_mfs _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
