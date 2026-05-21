// SPDX-License-Identifier: Apache-2.0
// Android table_view handler. android.widget.ListView with a flat
// ArrayAdapter<String> built from "▾ Section / row" entries. Real
// sectioned rendering with isEnabled() per-position (to make headers
// non-tappable) is a v2 enhancement.

#ifndef MPAPP_HANDLERS_ANDROID_TABLE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_TABLE_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../table_view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class table_view_handler<platform::android> {
public:
    table_view_handler();
    ~table_view_handler();

    table_view_handler(const table_view_handler&)            = delete;
    table_view_handler& operator=(const table_view_handler&) = delete;
    table_view_handler(table_view_handler&&)                 = delete;
    table_view_handler& operator=(table_view_handler&&)      = delete;

    void map_sections(table_view& tv);
    void map_row_height(table_view& tv);

    jobject native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<table_section_data>& sections);
    void apply_row_height(int h);

    struct sec_cb_t {
        table_view_handler<platform::android>* self;
        void operator()(const std::vector<table_section_data>& v) const { self->rebuild_items(v); }
    };
    struct rh_cb_t {
        table_view_handler<platform::android>* self;
        void operator()(int v) const { self->apply_row_height(v); }
    };

    jobject native_ = nullptr;

    sec_cb_t sec_cb_{this};
    rh_cb_t  rh_cb_{this};
    signal_slot<const std::vector<table_section_data>&> sec_slot_{};
    signal_slot<const int&>                              rh_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_TABLE_VIEW_HANDLER_HPP
