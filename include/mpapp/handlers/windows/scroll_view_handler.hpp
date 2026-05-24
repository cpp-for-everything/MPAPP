// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_scroll_view handler — wraps
// mux::Controls::ScrollViewer.

#ifndef MPAPP_HANDLERS_WINDOWS_SCROLL_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_SCROLL_VIEW_HANDLER_HPP

#include <memory>

#include "../../platform.hpp"
#include "../../internal/basic_scroll_view.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp::internal {

template <>
class scroll_view_handler<platform::windows> {
public:
    scroll_view_handler();
    ~scroll_view_handler();

    scroll_view_handler(const scroll_view_handler&)            = delete;
    scroll_view_handler& operator=(const scroll_view_handler&) = delete;
    scroll_view_handler(scroll_view_handler&&)                 = delete;
    scroll_view_handler& operator=(scroll_view_handler&&)      = delete;

    void map_content(basic_scroll_view& s);
    void map_orientation(basic_scroll_view& s);

    // Convenience: assign a non-owning child to basic_scroll_view.content
    // without forcing the caller to manage a real shared_ptr.
    void bind_content(basic_scroll_view& s, view& child);

    winrt::Microsoft::UI::Xaml::Controls::ScrollViewer&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ScrollViewer& native() const noexcept { return native_; }

private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_orientation(scroll_orientation o);

    struct content_cb_t {
        scroll_view_handler<platform::windows>* self = nullptr;
        void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); }
    };
    struct orient_cb_t {
        scroll_view_handler<platform::windows>* self = nullptr;
        void operator()(scroll_orientation o) const { self->apply_orientation(o); }
    };

    winrt::Microsoft::UI::Xaml::Controls::ScrollViewer native_{nullptr};
    basic_scroll_view*                                       bound_ = nullptr;

    content_cb_t                                  content_cb_{this};
    orient_cb_t                                   orient_cb_{this};
    signal_slot<std::shared_ptr<view> const&>     content_slot_{};
    signal_slot<const scroll_orientation&>        orient_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_SCROLL_VIEW_HANDLER_HPP
