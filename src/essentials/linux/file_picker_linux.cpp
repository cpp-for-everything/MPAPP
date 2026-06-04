// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// GTK4 GtkFileDialog implementation of `mpapp::linux_file_picker`.
// All GTK/GDK/GLib headers are confined to this translation unit; no GTK
// types leak into the public header.
//
// Synchronous bridging strategy:
//   GtkFileDialog is async-only in GTK 4.10+. To satisfy the synchronous
//   mpapp::file_picker interface we spin a nested GMainLoop:
//     1. Create a GMainLoop on the thread-default GLib main context.
//     2. Call gtk_file_dialog_open / gtk_file_dialog_open_multiple, passing
//        a GAsyncReadyCallback that stores the GAsyncResult and quits the loop.
//     3. Run the loop — this pumps GLib/GDK events until the callback fires.
//     4. Call the matching _finish function to obtain the GFile(s).
//     5. Convert to file_result and return.
//
//   This pattern is safe when called from a thread that owns a running GLib
//   main context (i.e. the GTK main thread). In headless / CI environments
//   where gdk_display_get_default() returns null the whole operation is
//   skipped to avoid crashes.
//
// File-type filtering:
//   pick_options::file_types entries are interpreted as:
//     '.' prefix        → gtk_file_filter_add_suffix  (e.g. ".png")
//     contains '/'      → gtk_file_filter_add_mime_type (e.g. "image/png")
//     any other string  → gtk_file_filter_add_pattern (e.g. "*.png")
//
// MIME-type detection:
//   g_content_type_guess() is used to infer the MIME type of a returned file.
//   It is a best-effort, extension-based heuristic; it returns "application/
//   octet-stream" when the type cannot be determined. An empty string is stored
//   when g_content_type_guess returns null.

#include "mpapp/essentials/linux/file_picker_linux.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <algorithm>
#include <string>
#include <vector>

#include <gtk/gtk.h>
#include <gio/gio.h>

namespace {

// ---------------------------------------------------------------------------
// Callback state for the nested-GMainLoop bridging
// ---------------------------------------------------------------------------

// Single-file async state.
struct OpenState {
    GMainLoop*    loop   = nullptr;
    GAsyncResult* result = nullptr; // not owned; we ref it ourselves
};

// Multi-file async state.
struct OpenMultipleState {
    GMainLoop*    loop   = nullptr;
    GAsyncResult* result = nullptr;
};

// GAsyncReadyCallback for gtk_file_dialog_open.
// Stores a ref to the result and quits the nested loop.
extern "C" void on_open_done(GObject*      /*source*/,
                              GAsyncResult* async_result,
                              gpointer      user_data) noexcept
{
    auto* state = static_cast<OpenState*>(user_data);
    if (state == nullptr) {
        return;
    }
    // Hold a reference so the result survives until we call _finish.
    state->result = async_result;
    g_object_ref(async_result);
    g_main_loop_quit(state->loop);
}

// GAsyncReadyCallback for gtk_file_dialog_open_multiple.
extern "C" void on_open_multiple_done(GObject*      /*source*/,
                                       GAsyncResult* async_result,
                                       gpointer      user_data) noexcept
{
    auto* state = static_cast<OpenMultipleState*>(user_data);
    if (state == nullptr) {
        return;
    }
    state->result = async_result;
    g_object_ref(async_result);
    g_main_loop_quit(state->loop);
}

// ---------------------------------------------------------------------------
// Helper: build a file_result from a GFile
// ---------------------------------------------------------------------------

[[nodiscard]] mpapp::file_result file_result_from_gfile(GFile* gfile)
{
    mpapp::file_result res;

    if (gfile == nullptr) {
        return res;
    }

    // full_path: absolute filesystem path (UTF-8 on Linux).
    char* path = g_file_get_path(gfile);
    if (path != nullptr) {
        res.full_path = path;
        g_free(path);
    }

    // file_name: basename component.
    char* basename = g_file_get_basename(gfile);
    if (basename != nullptr) {
        res.file_name = basename;
        g_free(basename);
    }

    // content_type: best-effort MIME type via GLib content-type guessing.
    if (!res.full_path.empty()) {
        gboolean uncertain    = FALSE;
        char*    content_type = g_content_type_guess(
            res.full_path.c_str(), nullptr, 0, &uncertain);
        if (content_type != nullptr) {
            // Convert the GLib content-type token to a MIME type string.
            char* mime = g_content_type_get_mime_type(content_type);
            if (mime != nullptr) {
                res.content_type = mime;
                g_free(mime);
            }
            g_free(content_type);
        }
    }

    return res;
}

// ---------------------------------------------------------------------------
// Helper: build a GtkFileFilter from pick_options::file_types
// ---------------------------------------------------------------------------
// Returns a new GtkFileFilter (caller owns a reference) or nullptr when
// file_types is empty (meaning: show all files).

[[nodiscard]] GtkFileFilter*
build_filter(const std::vector<std::string>& file_types)
{
    if (file_types.empty()) {
        return nullptr;
    }

    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Allowed files");

    for (const std::string& ft : file_types) {
        if (ft.empty()) {
            continue;
        }
        if (ft.front() == '.') {
            // Suffix entry, e.g. ".png" → add as a suffix filter.
            // gtk_file_filter_add_suffix expects the extension without the dot.
            const std::string suffix = ft.substr(1);
            if (!suffix.empty()) {
                gtk_file_filter_add_suffix(filter, suffix.c_str());
            }
        } else if (ft.find('/') != std::string::npos) {
            // Contains '/' → MIME type, e.g. "image/png".
            gtk_file_filter_add_mime_type(filter, ft.c_str());
        } else {
            // Anything else → glob pattern, e.g. "*.png".
            gtk_file_filter_add_pattern(filter, ft.c_str());
        }
    }

    return filter; // caller must g_object_unref when done
}

// ---------------------------------------------------------------------------
// Helper: configure and return a GtkFileDialog (caller owns reference)
// ---------------------------------------------------------------------------

[[nodiscard]] GtkFileDialog*
create_dialog(const mpapp::pick_options& options)
{
    GtkFileDialog* dialog = gtk_file_dialog_new();

    // Set dialog title when provided.
    if (!options.title.empty()) {
        gtk_file_dialog_set_title(dialog, options.title.c_str());
    }

    // Apply file-type filter when file_types is non-empty.
    GtkFileFilter* filter = build_filter(options.file_types);
    if (filter != nullptr) {
        // GTK4 GtkFileDialog accepts a GListModel of GtkFileFilter objects.
        GListStore* store = g_list_store_new(GTK_TYPE_FILE_FILTER);
        g_list_store_append(store, filter);
        g_object_unref(filter);
        gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(store));
        gtk_file_dialog_set_default_filter(dialog, GTK_FILE_FILTER(
            g_list_model_get_item(G_LIST_MODEL(store), 0)));
        g_object_unref(store);
    }

    return dialog; // caller must g_object_unref
}

// Compile-time verification that the class is a complete type.
static_assert(sizeof(mpapp::linux_file_picker) > 0,
              "linux_file_picker must be a complete type");

} // anonymous namespace

namespace mpapp {

// ---------------------------------------------------------------------------
// linux_file_picker::pick
// ---------------------------------------------------------------------------

std::optional<file_result>
linux_file_picker::pick(const pick_options& options)
{
    // Guard: no display → headless / CI environment, degrade gracefully.
    GdkDisplay* display = gdk_display_get_default();
    if (display == nullptr) {
        return std::nullopt;
    }

    GtkFileDialog* dialog = create_dialog(options);

    // Set up the nested GMainLoop bridge.
    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
    OpenState state{ loop, nullptr };

    // gtk_file_dialog_open is asynchronous; the callback quits the loop.
    gtk_file_dialog_open(dialog,
                         /*parent=*/nullptr,
                         /*cancellable=*/nullptr,
                         on_open_done,
                         &state);

    // Spin the main loop until on_open_done fires.
    g_main_loop_run(loop);
    g_main_loop_unref(loop);

    std::optional<file_result> result = std::nullopt;

    if (state.result != nullptr) {
        GError* error = nullptr;
        GFile*  gfile = gtk_file_dialog_open_finish(dialog,
                                                     state.result,
                                                     &error);
        if (gfile != nullptr) {
            result = file_result_from_gfile(gfile);
            g_object_unref(gfile);
        }
        if (error != nullptr) {
            // Cancelled or error — return nullopt (already the default).
            g_error_free(error);
        }
        g_object_unref(state.result);
    }

    g_object_unref(dialog);
    return result;
}

// ---------------------------------------------------------------------------
// linux_file_picker::pick_multiple
// ---------------------------------------------------------------------------

std::vector<file_result>
linux_file_picker::pick_multiple(const pick_options& options)
{
    // Guard: no display → headless / CI environment, degrade gracefully.
    GdkDisplay* display = gdk_display_get_default();
    if (display == nullptr) {
        return {};
    }

    GtkFileDialog* dialog = create_dialog(options);

    // Set up the nested GMainLoop bridge.
    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
    OpenMultipleState state{ loop, nullptr };

    gtk_file_dialog_open_multiple(dialog,
                                   /*parent=*/nullptr,
                                   /*cancellable=*/nullptr,
                                   on_open_multiple_done,
                                   &state);

    g_main_loop_run(loop);
    g_main_loop_unref(loop);

    std::vector<file_result> results;

    if (state.result != nullptr) {
        GError*     error = nullptr;
        GListModel* model = gtk_file_dialog_open_multiple_finish(dialog,
                                                                   state.result,
                                                                   &error);
        if (model != nullptr) {
            const guint count = g_list_model_get_n_items(model);
            results.reserve(static_cast<std::size_t>(count));
            for (guint i = 0; i < count; ++i) {
                GFile* gfile = G_FILE(g_list_model_get_item(model, i));
                if (gfile != nullptr) {
                    results.push_back(file_result_from_gfile(gfile));
                    g_object_unref(gfile);
                }
            }
            g_object_unref(model);
        }
        if (error != nullptr) {
            // Cancelled or error — return empty vector (already the default).
            g_error_free(error);
        }
        g_object_unref(state.result);
    }

    g_object_unref(dialog);
    return results;
}

} // namespace mpapp

#endif // defined(__linux__) && !defined(__ANDROID__)
