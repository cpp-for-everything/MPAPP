// SPDX-License-Identifier: Apache-2.0
// GTK4 table_view handler. GtkListBox in GtkScrolledWindow with
// gtk_list_box_set_header_func setting section headers as
// non-selectable rows above each section.

#ifndef MPAPP_HANDLERS_LINUX_TABLE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_TABLE_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../table_view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class table_view_handler<platform::linux_> {
public:
    table_view_handler();
    ~table_view_handler();

    table_view_handler(const table_view_handler&)            = delete;
    table_view_handler& operator=(const table_view_handler&) = delete;
    table_view_handler(table_view_handler&&)                 = delete;
    table_view_handler& operator=(table_view_handler&&)      = delete;

    void map_sections(table_view& tv);
    void map_typed_sections(table_view& tv);
    void map_row_height(table_view& tv);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<table_section_data>& sections);
    void rebuild_typed(const std::vector<table_section_typed>& sections);
    void apply_row_height(int h);
    void rebuild_active();

    struct sec_cb_t {
        table_view_handler<platform::linux_>* self;
        void operator()(const std::vector<table_section_data>&) const { self->rebuild_active(); }
    };
    struct typed_cb_t {
        table_view_handler<platform::linux_>* self;
        void operator()(const std::vector<table_section_typed>&) const { self->rebuild_active(); }
    };
    struct rh_cb_t {
        table_view_handler<platform::linux_>* self;
        void operator()(int v) const { self->apply_row_height(v); }
    };

    void* native_   = nullptr;  // GtkScrolledWindow*
    void* list_box_ = nullptr;  // GtkListBox*
    bool  tap_wired_ = false;
    table_view* bound_ = nullptr;

    sec_cb_t   sec_cb_{this};
    typed_cb_t typed_cb_{this};
    rh_cb_t    rh_cb_{this};
    signal_slot<const std::vector<table_section_data>&>  sec_slot_{};
    signal_slot<const std::vector<table_section_typed>&> typed_slot_{};
    signal_slot<const int&>                              rh_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_TABLE_VIEW_HANDLER_HPP
