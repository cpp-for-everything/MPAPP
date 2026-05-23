// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 page handler — wraps a `mux::Controls::Page`
// containing a 2-row `Grid`:
//   - row 0 ("Auto"): a `TextBlock` carrying `title`
//   - row 1 ("*"):    a single-child host for `content`
// `is_busy` toggles a translucent overlay above the content host. The
// `mux::Controls::Page` XAML control has no first-class Title slot
// (that's a `NavigationView` / `Frame` concern) so the title chrome is
// hand-rolled inside the Grid — same pattern as
// `content_page_handler<platform::windows>`.

#ifndef MPAPP_HANDLERS_WINDOWS_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_PAGE_HANDLER_HPP

#include <string>

#include "../../page.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class page_handler<platform::windows> {
public:
    page_handler();
    ~page_handler();

    page_handler(const page_handler&)            = delete;
    page_handler& operator=(const page_handler&) = delete;
    page_handler(page_handler&&)                 = delete;
    page_handler& operator=(page_handler&&)      = delete;

    void map_title(page& p);
    void map_content(page& p);
    void map_is_busy(page& p);

    // Convenience for tests / spike: assign a non-owning child.
    void bind_content(page& p, view& child);

    winrt::Microsoft::UI::Xaml::Controls::Grid&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Grid& native() const noexcept { return native_; }

private:
    void apply_title(const std::string& v);
    void apply_content(view* v);
    void apply_is_busy(bool v);

    struct title_cb_t   { page_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_title(v); } };
    struct content_cb_t { page_handler<platform::windows>* self; void operator()(view* v) const { self->apply_content(v); } };
    struct busy_cb_t    { page_handler<platform::windows>* self; void operator()(bool v) const { self->apply_is_busy(v); } };

    // `native_` is a Grid (title in row 0, content host in row 1) — NOT a
    // `muxc::Page`. Page is designed for Frame navigation; nesting Page
    // inside another container (e.g. the navigation_page_handler's
    // ContentControl, or the shell_handler's content host) triggers a
    // late layout-pass exception. The page's chrome (title + content
    // host + busy ring) lives inside this Grid directly.
    winrt::Microsoft::UI::Xaml::Controls::Grid           native_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::TextBlock      title_text_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::ContentControl content_host_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::ProgressRing   busy_ring_{nullptr};

    title_cb_t                                title_cb_{this};
    content_cb_t                              content_cb_{this};
    busy_cb_t                                 busy_cb_{this};
    signal_slot<const std::string&>           title_slot_{};
    signal_slot<view* const&>                 content_slot_{};
    signal_slot<const bool&>                  busy_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_PAGE_HANDLER_HPP
