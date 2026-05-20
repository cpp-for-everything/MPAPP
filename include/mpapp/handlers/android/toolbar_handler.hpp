// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android toolbar handler — wraps `android.widget.Toolbar`
// + per-item `android.view.MenuItem` entries added via the toolbar's
// `Menu` (Toolbar.getMenu().add(...) / .clear()).

#ifndef MPAPP_HANDLERS_ANDROID_TOOLBAR_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_TOOLBAR_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../toolbar.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class toolbar_handler<platform::android> {
public:
    toolbar_handler();
    ~toolbar_handler();
    toolbar_handler(const toolbar_handler&)            = delete;
    toolbar_handler& operator=(const toolbar_handler&) = delete;

    void map_items(toolbar& t);
    void map_title(toolbar& t);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_items(const std::vector<toolbar_item>& v);
    void apply_title(const std::string& v);

    struct items_cb_t { toolbar_handler<platform::android>* self; void operator()(const std::vector<toolbar_item>& v) const { self->apply_items(v); } };
    struct title_cb_t { toolbar_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_title(v); } };

    jobject native_ = nullptr;  // android.widget.Toolbar global ref

    items_cb_t                                          items_cb_{this};
    title_cb_t                                          title_cb_{this};
    signal_slot<std::vector<toolbar_item> const&>       items_slot_{};
    signal_slot<const std::string&>                     title_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_TOOLBAR_HANDLER_HPP
