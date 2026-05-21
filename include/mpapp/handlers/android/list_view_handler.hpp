// SPDX-License-Identifier: Apache-2.0
// Android list_view handler. Wraps android.widget.ListView with an
// ArrayAdapter<String> bound to items_source.

#ifndef MPAPP_HANDLERS_ANDROID_LIST_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_LIST_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../list_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class list_view_handler<platform::android> {
public:
    list_view_handler();
    ~list_view_handler();

    list_view_handler(const list_view_handler&)            = delete;
    list_view_handler& operator=(const list_view_handler&) = delete;
    list_view_handler(list_view_handler&&)                 = delete;
    list_view_handler& operator=(list_view_handler&&)      = delete;

    void map_items_source(list_view& lv);
    void map_selected_index(list_view& lv);

    jobject native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void apply_selection(int idx);

    struct items_cb_t {
        list_view_handler<platform::android>* self;
        void operator()(const std::vector<std::string>& v) const { self->rebuild_items(v); }
    };
    struct sel_cb_t {
        list_view_handler<platform::android>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };

    jobject native_ = nullptr;  // android.widget.ListView
    list_view* bound_ = nullptr;

    items_cb_t items_cb_{this};
    sel_cb_t   sel_cb_{this};
    signal_slot<const std::vector<std::string>&> items_slot_{};
    signal_slot<const int&>                       sel_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_LIST_VIEW_HANDLER_HPP
