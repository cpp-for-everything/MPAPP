// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_content_page handler — wraps a
// `mux::Controls::Border` (which honors Padding) containing a 2-row
// `Grid`:
//   - row 0 ("Auto"): a `TextBlock` carrying `title`
//   - row 1 ("*"):    a single-child host for `content`
//
// Earlier revisions wrapped `native_` in `mux::Controls::Page`. Page
// is the WinUI navigation-host control and misbehaves when nested
// inside another container's `ContentControl` or pane slot — the
// late-layout-pass `STATUS_APPLICATION_INTERNAL_EXCEPTION` we
// debugged in T-0014 hits basic_content_page the same way it hit
// basic_navigation_page. Switched to Border (which is a plain UIElement
// that wraps a single child + has a Padding property) so
// basic_content_page can be safely nested anywhere a UIElement is
// accepted. `padding` now writes to `Border.Padding`.

#ifndef MPAPP_HANDLERS_WINDOWS_CONTENT_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_CONTENT_PAGE_HANDLER_HPP

#include <memory>
#include <string>

#include "../../internal/basic_content_page.hpp"
#include "../../layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class content_page_handler<platform::windows> {
public:
    content_page_handler();
    ~content_page_handler();

    content_page_handler(const content_page_handler&)            = delete;
    content_page_handler& operator=(const content_page_handler&) = delete;
    content_page_handler(content_page_handler&&)                 = delete;
    content_page_handler& operator=(content_page_handler&&)      = delete;

    void map_title(basic_content_page& p);
    void map_content(basic_content_page& p);
    void map_padding(basic_content_page& p);

    // Convenience for tests / spike: assign a non-owning child.
    void bind_content(basic_content_page& p, view& child);

    winrt::Microsoft::UI::Xaml::Controls::Border&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Border& native() const noexcept { return native_; }

private:
    void apply_title(const std::string& v);
    void apply_content(const std::shared_ptr<view>& v);
    void apply_padding(const thickness& t);

    struct title_cb_t   { content_page_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_title(v); } };
    struct content_cb_t { content_page_handler<platform::windows>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct padding_cb_t { content_page_handler<platform::windows>* self; void operator()(const thickness& t) const { self->apply_padding(t); } };

    // `native_` is a Border wrapping the inner Grid — Border has
    // Padding, accepts a single child, and lives happily inside a
    // ContentControl / Frame / Page slot in another container. See
    // header comment for the Page-nesting bug that drove this.
    winrt::Microsoft::UI::Xaml::Controls::Border       native_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::Grid         grid_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::TextBlock    title_text_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::ContentControl content_host_{nullptr};

    title_cb_t                                title_cb_{this};
    content_cb_t                              content_cb_{this};
    padding_cb_t                              padding_cb_{this};
    signal_slot<const std::string&>           title_slot_{};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const thickness&>             padding_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_CONTENT_PAGE_HANDLER_HPP
