// SPDX-License-Identifier: Apache-2.0
// GTK4/GLib real main-thread dispatcher implementation.

#include "mpapp/handlers/linux/glib_dispatcher.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <chrono>
#include <functional>
#include <utility>

#include <glib.h>

#include "mpapp/executor.hpp"

namespace mpapp::internal {

namespace {

// One-shot GSource trampoline: invokes the heap-owned std::function once and
// frees it. Returns G_SOURCE_REMOVE so the idle/timeout source is removed
// after firing (matches dispatcher::post semantics — fire exactly once).
gboolean run_once(gpointer data) {
    auto* fn = static_cast<std::function<void()>*>(data);
    if (fn != nullptr && *fn) {
        (*fn)();
    }
    return G_SOURCE_REMOVE;
}

void destroy_fn(gpointer data) {
    delete static_cast<std::function<void()>*>(data);
}

} // namespace

class glib_dispatcher final : public ::mpapp::dispatcher {
public:
    void post(std::function<void()> work) override {
        auto* p = new std::function<void()>(std::move(work));
        // G_PRIORITY_DEFAULT (not _IDLE) so posted UI work isn't starved
        // behind GTK redraw idles. destroy_fn frees the closure even if the
        // source is removed before firing.
        g_idle_add_full(G_PRIORITY_DEFAULT, &run_once, p, &destroy_fn);
    }

    void post_after(std::chrono::steady_clock::duration delay,
                    std::function<void()> work) override {
        auto* p = new std::function<void()>(std::move(work));
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delay).count();
        if (ms < 0) ms = 0;
        g_timeout_add_full(G_PRIORITY_DEFAULT, static_cast<guint>(ms),
                           &run_once, p, &destroy_fn);
    }
};

} // namespace mpapp::internal

namespace mpapp::detail {

void install_glib_main_dispatcher() {
    static ::mpapp::internal::glib_dispatcher inst;
    ::mpapp::install_main_dispatcher(&inst);
}

} // namespace mpapp::detail

#endif // __linux__ && !__ANDROID__
