// SPDX-License-Identifier: Apache-2.0
// WinUI 3 collection_view handler. Same wrap-platform-recycler shape
// as list_view per ADR-0020. Honors:
//   * selection_mode (None/Single/Multiple)
//   * layout (vertical_list = mux::ListView,
//             vertical_grid = mux::GridView)
//
// The inner widget swaps when layout changes — kept inside an outer
// mux::Border (native_) so the ADR-0013 dispatch handle is stable.
// horizontal_list / horizontal_grid still degrade to vertical_list
// for v1 (deferred to a future ItemsPanelTemplate pass).

#ifndef MPAPP_HANDLERS_WINDOWS_COLLECTION_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_COLLECTION_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../collection_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class collection_view_handler<platform::windows> {
public:
    collection_view_handler();
    ~collection_view_handler();

    collection_view_handler(const collection_view_handler&)            = delete;
    collection_view_handler& operator=(const collection_view_handler&) = delete;
    collection_view_handler(collection_view_handler&&)                 = delete;
    collection_view_handler& operator=(collection_view_handler&&)      = delete;

    void map_items_source(collection_view& cv);
    void map_typed_items(collection_view& cv);
    void map_selected_index(collection_view& cv);
    void map_selection_mode(collection_view& cv);
    void map_layout(collection_view& cv);

    winrt::Microsoft::UI::Xaml::Controls::Border&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Border& native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void rebuild_typed(const std::vector<view*>& v);
    void apply_selection(int idx);
    void apply_selection_mode(collection_selection_mode m);
    void apply_layout(collection_layout l);
    void wire_inner_selection_changed();
    void rebuild_active();   // dispatches typed vs flat based on which is non-empty

    struct items_cb_t {
        collection_view_handler<platform::windows>* self;
        void operator()(const std::vector<std::string>&) const { self->rebuild_active(); }
    };
    struct typed_cb_t {
        collection_view_handler<platform::windows>* self;
        void operator()(const std::vector<view*>&) const { self->rebuild_active(); }
    };
    struct sel_cb_t {
        collection_view_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };
    struct mode_cb_t {
        collection_view_handler<platform::windows>* self;
        void operator()(collection_selection_mode m) const { self->apply_selection_mode(m); }
    };
    struct layout_cb_t {
        collection_view_handler<platform::windows>* self;
        void operator()(collection_layout l) const { self->apply_layout(l); }
    };
    struct materialized_cb_t {
        collection_view_handler<platform::windows>* self;
        void operator()() const { self->rebuild_active(); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Border       native_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::ListViewBase inner_{nullptr};
    winrt::event_token                                 selection_token_{};
    collection_view* bound_ = nullptr;
    bool             suppress_selection_event_ = false;

    items_cb_t        items_cb_{this};
    typed_cb_t        typed_cb_{this};
    sel_cb_t          sel_cb_{this};
    mode_cb_t         mode_cb_{this};
    layout_cb_t       layout_cb_{this};
    materialized_cb_t materialized_cb_{this};
    signal_slot<const std::vector<std::string>&>          items_slot_{};
    signal_slot<const std::vector<view*>&>                typed_slot_{};
    signal_slot<const int&>                                sel_slot_{};
    signal_slot<const collection_selection_mode&>          mode_slot_{};
    signal_slot<const collection_layout&>                  layout_slot_{};
    signal_slot<>                                          materialized_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_COLLECTION_VIEW_HANDLER_HPP
