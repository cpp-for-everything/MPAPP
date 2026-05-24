// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_date_picker handler implementation.

#include "mpapp/handlers/linux/date_picker_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp::internal {

date_picker_handler<platform::linux_>::date_picker_handler() {
    native_ = gtk_calendar_new();
}

date_picker_handler<platform::linux_>::~date_picker_handler() = default;

void date_picker_handler<platform::linux_>::apply_date(const date_value& v) {
    if (native_ == nullptr) return;
    GtkCalendar* cal = GTK_CALENDAR(static_cast<GtkWidget*>(native_));
    GDateTime* dt = g_date_time_new_local(v.year, v.month, v.day, 0, 0, 0.0);
    if (dt != nullptr) {
        gtk_calendar_select_day(cal, dt);
        g_date_time_unref(dt);
    }
}

void date_picker_handler<platform::linux_>::map_date(basic_date_picker& p) {
    apply_date(p.date.get());
    p.date.changed.subscribe(date_slot_, date_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_date_picker so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_date_picker.hpp"

namespace {

GtkWidget* dispatch_date_picker(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_date_picker*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_date_picker); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
