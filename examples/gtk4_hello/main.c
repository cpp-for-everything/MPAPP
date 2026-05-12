/* SPDX-License-Identifier: Apache-2.0
 *
 * T-0007 — WSLg GTK4 hello window.
 *
 * Minimal GTK4 application that opens a single window with a greeting
 * label. Built only on UNIX (non-Apple) hosts; intended to be run from
 * a WSL2 Ubuntu guest so the window surfaces on the Windows desktop
 * via WSLg.
 *
 * Build (inside WSL2 Ubuntu, after `apt install libgtk-4-dev`):
 *     cmake -S . -B build-wsl -G Ninja -DMPAPP_BUILD_EXAMPLES=ON
 *     cmake --build build-wsl --target gtk4_hello
 *     ./build-wsl/examples/gtk4_hello/gtk4_hello
 */

#include <gtk/gtk.h>

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "MPAPP - WSLg GTK4 hello");
    gtk_window_set_default_size(GTK_WINDOW(window), 480, 240);

    GtkWidget *label = gtk_label_new(
        "Hello from MPAPP - cross-built from Windows, running on WSLg.");
    gtk_window_set_child(GTK_WINDOW(window), label);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app =
        gtk_application_new("io.mpapp.hello", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
