// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_flyout_page handler. Uses `mux::Controls::SplitView` —
// Pane = flyout, Content = detail, IsPaneOpen mirrors is_presented.

#ifndef MPAPP_HANDLERS_WINDOWS_FLYOUT_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_FLYOUT_PAGE_HANDLER_HPP

#include "../../internal/basic_flyout_page.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class flyout_page_handler<platform::windows> {
public:
    flyout_page_handler();
    ~flyout_page_handler();

    flyout_page_handler(const flyout_page_handler&)            = delete;
    flyout_page_handler& operator=(const flyout_page_handler&) = delete;
    flyout_page_handler(flyout_page_handler&&)                 = delete;
    flyout_page_handler& operator=(flyout_page_handler&&)      = delete;

    void map_flyout(basic_flyout_page& fp);
    void map_detail(basic_flyout_page& fp);
    void map_is_presented(basic_flyout_page& fp);

    winrt::Microsoft::UI::Xaml::Controls::SplitView&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::SplitView& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_flyout_page& /*x*/) noexcept {}


private:
    void apply_flyout(basic_page* p);
    void apply_detail(basic_page* p);
    void apply_is_presented(bool v);

    struct flyout_cb_t {
        flyout_page_handler<platform::windows>* self;
        void operator()(basic_page* p) const { self->apply_flyout(p); }
    };
    struct detail_cb_t {
        flyout_page_handler<platform::windows>* self;
        void operator()(basic_page* p) const { self->apply_detail(p); }
    };
    struct presented_cb_t {
        flyout_page_handler<platform::windows>* self;
        void operator()(bool v) const { self->apply_is_presented(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::SplitView native_{nullptr};

    flyout_cb_t    flyout_cb_{this};
    detail_cb_t    detail_cb_{this};
    presented_cb_t presented_cb_{this};
    signal_slot<basic_page* const&> flyout_slot_{};
    signal_slot<basic_page* const&> detail_slot_{};
    signal_slot<const bool&>  presented_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_FLYOUT_PAGE_HANDLER_HPP
