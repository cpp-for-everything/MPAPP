// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 navigation_page handler.
//
// Wraps a `mux::Controls::Page` containing a 2-row Grid:
//   - row 0 (Auto): a horizontal bar with the back button + the current
//                   page's title.
//   - row 1 (*):    a single-child host swapped to the top of the
//                   page_stack on every appear signal.
//
// Subscribes to the cross-platform `page_stack` lifecycle signals
// (per ADR-0014) and pulls native widgets out of the ADR-0013 dispatch
// registry — same pattern as `content_view_handler<windows>`.

#ifndef MPAPP_HANDLERS_WINDOWS_NAVIGATION_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_NAVIGATION_PAGE_HANDLER_HPP

#include <cstddef>
#include <string>

#include "../../navigation_page.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class navigation_page_handler<platform::windows> {
public:
    navigation_page_handler();
    ~navigation_page_handler();

    navigation_page_handler(const navigation_page_handler&)            = delete;
    navigation_page_handler& operator=(const navigation_page_handler&) = delete;
    navigation_page_handler(navigation_page_handler&&)                 = delete;
    navigation_page_handler& operator=(navigation_page_handler&&)      = delete;

    // Wire the handler to the navigation_page. Subscribes to lifecycle
    // signals and seeds the content with the current top (if any).
    void map_stack(navigation_page& np);

    // Native accessor (returned to dispatch registry as a UIElement).
    winrt::Microsoft::UI::Xaml::Controls::Page&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Page& native() const noexcept { return native_; }

private:
    void apply_top(view* new_top);
    void apply_title(const std::string& v);
    void apply_back_visibility(std::size_t depth);

    struct did_appear_cb {
        navigation_page_handler<platform::windows>* self;
        void operator()(view* v) const { self->apply_top(v); }
    };
    struct depth_cb {
        navigation_page_handler<platform::windows>* self;
        void operator()(std::size_t d) const { self->apply_back_visibility(d); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Page           native_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::Grid           grid_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::StackPanel     bar_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::Button         back_button_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::TextBlock      title_text_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::ContentControl content_host_{nullptr};

    navigation_page* bound_ = nullptr;

    did_appear_cb        did_appear_cb_{this};
    depth_cb             depth_cb_{this};
    signal_slot<view*>            did_appear_slot_{};
    signal_slot<const std::size_t&> depth_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_NAVIGATION_PAGE_HANDLER_HPP
