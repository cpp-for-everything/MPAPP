// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_navigation_page handler implementation.

#include "mpapp/handlers/linux/navigation_page_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_page.hpp"

namespace mpapp::internal {

namespace {

void on_back_clicked(GtkButton* /*btn*/, gpointer user_data) {
    auto* np = static_cast<mpapp::internal::basic_navigation_page*>(user_data);
    if (np != nullptr && np->stack().depth() > 1) np->pop();
}

} // namespace

navigation_page_handler<platform::linux_>::navigation_page_handler() {
    native_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);

    bar_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    back_button_ = gtk_button_new_with_label("<");
    title_label_ = gtk_label_new("");
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(title_label_), TRUE);
    gtk_widget_set_halign(static_cast<GtkWidget*>(title_label_), GTK_ALIGN_START);

    gtk_widget_set_visible(static_cast<GtkWidget*>(back_button_), FALSE);

    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(bar_)), static_cast<GtkWidget*>(back_button_));
    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(bar_)), static_cast<GtkWidget*>(title_label_));

    content_host_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(content_host_), TRUE);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(content_host_), TRUE);

    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(native_)), static_cast<GtkWidget*>(bar_));
    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(native_)), static_cast<GtkWidget*>(content_host_));
}

navigation_page_handler<platform::linux_>::~navigation_page_handler() = default;

void navigation_page_handler<platform::linux_>::apply_top(view* new_top) {
    GtkBox* host = GTK_BOX(static_cast<GtkWidget*>(content_host_));
    if (current_child_ != nullptr) {
        gtk_box_remove(host, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }
    if (new_top != nullptr) {
        if (GtkWidget* w = detail::linux_dispatch::dispatch(new_top); w != nullptr) {
            gtk_box_append(host, w);
            current_child_ = w;
        }
    }
    if (auto* p = dynamic_cast<basic_page*>(new_top); p != nullptr) {
        apply_title(p->title.get());
    } else {
        apply_title("");
    }
}

void navigation_page_handler<platform::linux_>::apply_title(const std::string& v) {
    if (title_label_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(title_label_)), v.c_str());
}

void navigation_page_handler<platform::linux_>::apply_back_visibility(std::size_t depth) {
    if (back_button_ == nullptr) return;
    gtk_widget_set_visible(static_cast<GtkWidget*>(back_button_), depth > 1 ? TRUE : FALSE);
}

void navigation_page_handler<platform::linux_>::map_stack(basic_navigation_page& np) {
    bound_ = &np;
    apply_top(np.stack().top());
    apply_back_visibility(np.stack().depth());

    np.stack().page_did_appear.subscribe(did_appear_slot_, did_appear_cb_);
    np.stack_depth.changed.subscribe(depth_slot_, depth_cb_);

    g_signal_connect(static_cast<GtkWidget*>(back_button_), "clicked",
                     G_CALLBACK(on_back_clicked), &np);
}

void navigation_page_handler<platform::linux_>::map_gestures(basic_navigation_page& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_navigation_page(::mpapp::view* v) {
    if (auto* n = dynamic_cast<::mpapp::internal::basic_navigation_page*>(v); n && n->has_np_handler()) {
        return GTK_WIDGET(n->np_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_navigation_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
