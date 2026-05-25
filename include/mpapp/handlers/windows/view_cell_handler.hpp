// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_view_cell handler — wraps a Border with single-slot content
// resolved via ADR-0013 dispatch on the bound `content` Observable.

#ifndef MPAPP_HANDLERS_WINDOWS_VIEW_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_VIEW_CELL_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_view_cell.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class view_cell_handler<platform::windows> {
public:
    view_cell_handler();
    ~view_cell_handler();

    view_cell_handler(const view_cell_handler&)            = delete;
    view_cell_handler& operator=(const view_cell_handler&) = delete;
    view_cell_handler(view_cell_handler&&)                 = delete;
    view_cell_handler& operator=(view_cell_handler&&)      = delete;

    void map_content(basic_view_cell& c);

    winrt::Microsoft::UI::Xaml::Controls::Border&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Border& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_view_cell& /*x*/) noexcept {}


private:
    void apply_content(view* v);

    struct content_cb_t {
        view_cell_handler<platform::windows>* self;
        void operator()(view* v) const { self->apply_content(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Border native_{nullptr};

    content_cb_t content_cb_{this};
    signal_slot<view* const&> content_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_VIEW_CELL_HANDLER_HPP
