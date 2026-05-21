// SPDX-License-Identifier: Apache-2.0
// WinUI 3 collection_view handler. Same wrap-platform-recycler shape as
// list_view per ADR-0020. V1 honors selection_mode (None/Single/Multiple)
// and renders as a vertical list; horizontal + grid layouts are a
// follow-up tied to the layout enum.

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
    void map_selected_index(collection_view& cv);
    void map_selection_mode(collection_view& cv);

    winrt::Microsoft::UI::Xaml::Controls::ListView&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ListView& native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void apply_selection(int idx);
    void apply_selection_mode(collection_selection_mode m);

    struct items_cb_t {
        collection_view_handler<platform::windows>* self;
        void operator()(const std::vector<std::string>& v) const { self->rebuild_items(v); }
    };
    struct sel_cb_t {
        collection_view_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };
    struct mode_cb_t {
        collection_view_handler<platform::windows>* self;
        void operator()(collection_selection_mode m) const { self->apply_selection_mode(m); }
    };

    winrt::Microsoft::UI::Xaml::Controls::ListView native_{nullptr};
    collection_view* bound_ = nullptr;
    bool             suppress_selection_event_ = false;

    items_cb_t items_cb_{this};
    sel_cb_t   sel_cb_{this};
    mode_cb_t  mode_cb_{this};
    signal_slot<const std::vector<std::string>&>          items_slot_{};
    signal_slot<const int&>                                sel_slot_{};
    signal_slot<const collection_selection_mode&>          mode_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_COLLECTION_VIEW_HANDLER_HPP
