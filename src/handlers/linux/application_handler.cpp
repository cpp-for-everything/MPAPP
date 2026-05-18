// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 application handler implementation.

#include "mpapp/handlers/linux/application_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <atomic>

#include <gtk/gtk.h>

namespace mpapp::detail {

namespace {

// Singleton handoff between `gtk_run_app_impl` and the "activate"
// signal callback. GtkApplication's signal callback signature is fixed
// at `void(GtkApplication*, gpointer user_data)` so we use user_data
// to carry the launcher and the result struct.
struct gtk_launcher_handoff {
    gtk_application_launcher launcher{};
    mpapp::application*      out_app  = nullptr;
};

void on_activate(GtkApplication* /*app*/, gpointer user_data) {
    auto* h = static_cast<gtk_launcher_handoff*>(user_data);
    if (h == nullptr || h->launcher.construct == nullptr) {
        return;
    }
    h->out_app = h->launcher.construct();
    if (h->out_app != nullptr) {
        h->out_app->on_launch();
    }
}

} // namespace

int gtk_run_app_impl(const gtk_application_launcher& launcher,
                     int argc, char** argv,
                     mpapp::application*& out_app) {
    GtkApplication* app = gtk_application_new(
        "io.mpapp.spike", G_APPLICATION_DEFAULT_FLAGS);

    gtk_launcher_handoff handoff{};
    handoff.launcher = launcher;

    g_signal_connect(app, "activate",
                     G_CALLBACK(on_activate),
                     &handoff);

    const int rc = g_application_run(G_APPLICATION(app), argc, argv);

    out_app = handoff.out_app;
    g_object_unref(app);
    return rc;
}

} // namespace mpapp::detail

#endif // __linux__ && !__ANDROID__
