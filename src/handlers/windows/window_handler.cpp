// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — WinUI 3 window handler implementation.

#include "mpapp/handlers/windows/window_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "mpapp/activity_indicator.hpp"
#include "mpapp/border.hpp"
#include "mpapp/box_view.hpp"
#include "mpapp/date_picker.hpp"
#include "mpapp/image.hpp"
#include "mpapp/image_button.hpp"
#include "mpapp/picker.hpp"
#include "mpapp/time_picker.hpp"
#include "mpapp/progress_bar.hpp"
#include "mpapp/search_bar.hpp"
#include "mpapp/button.hpp"
#include "mpapp/check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/entry.hpp"
#include "mpapp/handlers/windows/activity_indicator_handler.hpp"
#include "mpapp/handlers/windows/border_handler.hpp"
#include "mpapp/handlers/windows/box_view_handler.hpp"
#include "mpapp/handlers/windows/date_picker_handler.hpp"
#include "mpapp/handlers/windows/image_handler.hpp"
#include "mpapp/handlers/windows/image_button_handler.hpp"
#include "mpapp/handlers/windows/picker_handler.hpp"
#include "mpapp/handlers/windows/time_picker_handler.hpp"
#include "mpapp/handlers/windows/progress_bar_handler.hpp"
#include "mpapp/handlers/windows/search_bar_handler.hpp"
#include "mpapp/handlers/windows/button_handler.hpp"
#include "mpapp/handlers/windows/check_box_handler.hpp"
#include "mpapp/handlers/windows/editor_handler.hpp"
#include "mpapp/handlers/windows/entry_handler.hpp"
#include "mpapp/handlers/windows/label_handler.hpp"
#include "mpapp/handlers/windows/radio_button_handler.hpp"
#include "mpapp/handlers/windows/scroll_view_handler.hpp"
#include "mpapp/handlers/windows/slider_handler.hpp"
#include "mpapp/handlers/windows/stack_layout_handler.hpp"
#include "mpapp/handlers/windows/stepper_handler.hpp"
#include "mpapp/handlers/windows/switch_handler.hpp"
#include "mpapp/label.hpp"
#include "mpapp/radio_button.hpp"
#include "mpapp/scroll_view.hpp"
#include "mpapp/slider.hpp"
#include "mpapp/stack_layout.hpp"
#include "mpapp/stepper.hpp"
#include "mpapp/switch_.hpp"

#include "winrt_strings.hpp"

namespace mpapp {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

window_handler<platform::windows>::window_handler() {
    native_ = mux::Window{};
}

window_handler<platform::windows>::~window_handler() {
    // The `Closed` event handler captures `bound_` (a window*); if the
    // user's window outlives the handler we'd be left with a dangling
    // pointer. Unregister explicitly.
    if (native_ != nullptr && closed_token_.value != 0) {
        try {
            native_.Closed(closed_token_);
        } catch (...) {
            // Destructors must not propagate across the interop edge.
        }
        closed_token_ = {};
    }
}

void window_handler<platform::windows>::apply_title(const std::string& v) {
    if (native_ != nullptr) {
        native_.Title(detail::to_hstring_utf8(v));
    }
}

void window_handler<platform::windows>::apply_content(view* v) {
    if (native_ == nullptr) {
        return;
    }
    if (v == nullptr) {
        native_.Content(nullptr);
        return;
    }
    // ADR-0013: registry first.
    if (auto el = detail::windows_dispatch::dispatch(v); el != nullptr) {
        native_.Content(el);
        return;
    }
    // All concrete widgets self-register via ADR-0013; the legacy
    // dynamic_cast chain has been removed.
}

void window_handler<platform::windows>::apply_is_visible(bool v) {
    if (native_ == nullptr) {
        return;
    }
    if (v) {
        native_.Activate();
        was_activated_ = true;
    } else if (was_activated_) {
        // Only call Close() if the window has been shown at least once.
        // Calling Close() on a never-activated mux::Window throws
        // 0x800710DD.
        native_.Close();
    }
}

void window_handler<platform::windows>::bind(window& w) {
    bound_ = &w;

    apply_title(w.title.get());
    w.title.changed.subscribe(title_slot_, title_cb_);

    apply_content(w.content.get());
    w.content.changed.subscribe(content_slot_, content_cb_);

    apply_is_visible(w.is_visible.get());
    w.is_visible.changed.subscribe(visible_slot_, visible_cb_);

    if (closed_token_.value == 0 && native_ != nullptr) {
        window* target = &w;
        closed_token_ = native_.Closed(
            [target](winrt::Windows::Foundation::IInspectable const&,
                     mux::WindowEventArgs const&) {
                target->closed.emit();
            });
    }
}

} // namespace mpapp

#endif // _WIN32
