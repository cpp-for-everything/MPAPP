// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 radio_button handler implementation.
//
// GTK4 unifies radio buttons under GtkCheckButton with `set_group`. We
// use a per-app group registry keyed on `group_name` so two
// radio_buttons sharing the same group_name attach to the same
// GtkCheckButton "leader" widget.

#include "mpapp/handlers/linux/radio_button_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <map>
#include <mutex>

#include <gtk/gtk.h>

namespace mpapp {

namespace {

// Per-process group leader registry. First radio_button to bind for a
// given group_name becomes the leader; subsequent ones attach to it
// via gtk_check_button_set_group.
std::map<std::string, GtkCheckButton*>& group_leaders() {
    static std::map<std::string, GtkCheckButton*> g;
    return g;
}
std::mutex& group_leaders_mutex() {
    static std::mutex m;
    return m;
}

struct toggled_ctx {
    radio_button*                          target;
    radio_button_handler<platform::linux_>* handler;
};

void on_toggled(GtkCheckButton* btn, gpointer user_data) {
    auto* ctx = static_cast<toggled_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr) return;
    const bool v = gtk_check_button_get_active(btn) == TRUE;
    if (ctx->target->is_checked.get() != v) {
        ctx->target->is_checked.set(v);
    }
}

} // namespace

radio_button_handler<platform::linux_>::radio_button_handler() {
    native_ = gtk_check_button_new();
}

radio_button_handler<platform::linux_>::~radio_button_handler() {
    if (native_ != nullptr && toggled_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    toggled_handler_id_);
        toggled_handler_id_ = 0;
    }
}

void radio_button_handler<platform::linux_>::apply_is_checked(bool v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    gtk_check_button_set_active(GTK_CHECK_BUTTON(static_cast<GtkWidget*>(native_)),
                                v ? TRUE : FALSE);
    suppress_echo_ = false;
}

void radio_button_handler<platform::linux_>::apply_group_name(const std::string& v) {
    if (native_ == nullptr || v.empty()) return;
    GtkCheckButton* leader = nullptr;
    {
        std::lock_guard<std::mutex> g{group_leaders_mutex()};
        auto& reg = group_leaders();
        auto it = reg.find(v);
        if (it == reg.end()) {
            reg[v] = GTK_CHECK_BUTTON(static_cast<GtkWidget*>(native_));
            return;  // we ARE the leader; nothing to link to.
        }
        leader = it->second;
    }
    if (leader != nullptr && leader != GTK_CHECK_BUTTON(static_cast<GtkWidget*>(native_))) {
        gtk_check_button_set_group(
            GTK_CHECK_BUTTON(static_cast<GtkWidget*>(native_)),
            leader);
    }
}

void radio_button_handler<platform::linux_>::map_is_checked(radio_button& r) {
    apply_is_checked(r.is_checked.get());
    r.is_checked.changed.subscribe(is_checked_slot_, is_checked_cb_);

    if (native_ == nullptr) return;
    auto* ctx = new toggled_ctx{&r, this};
    if (toggled_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    toggled_handler_id_);
    }
    toggled_handler_id_ = g_signal_connect_data(
        static_cast<GtkWidget*>(native_),
        "toggled",
        G_CALLBACK(on_toggled),
        ctx,
        +[](gpointer p, GClosure*) { delete static_cast<toggled_ctx*>(p); },
        static_cast<GConnectFlags>(0));
}

void radio_button_handler<platform::linux_>::map_group_name(radio_button& r) {
    apply_group_name(r.group_name.get());
    r.group_name.changed.subscribe(group_name_slot_, group_name_cb_);
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
