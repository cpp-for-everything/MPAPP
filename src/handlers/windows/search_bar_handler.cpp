// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 search_bar handler implementation.

#include "mpapp/handlers/windows/search_bar_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "winrt_strings.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

search_bar_handler<platform::windows>::search_bar_handler() {
    native_ = muxc::AutoSuggestBox{};
    // Search-affordance styling: a magnifier icon in the QueryButton slot.
    try {
        native_.QueryIcon(muxc::SymbolIcon{muxc::Symbol::Find});
    } catch (...) { /* leave default */ }
}

search_bar_handler<platform::windows>::~search_bar_handler() = default;

void search_bar_handler<platform::windows>::apply_text(const std::string& v) {
    if (native_ == nullptr) return;
    if (suppress_echo_) return;
    suppress_echo_ = true;
    try {
        native_.Text(detail::to_hstring_utf8(v));
    } catch (...) { /* ignore */ }
    suppress_echo_ = false;
}

void search_bar_handler<platform::windows>::apply_placeholder(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        native_.PlaceholderText(detail::to_hstring_utf8(v));
    } catch (...) {}
}

void search_bar_handler<platform::windows>::map_text(search_bar& s) {
    apply_text(s.text.get());
    s.text.changed.subscribe(text_slot_, text_cb_);
}
void search_bar_handler<platform::windows>::map_placeholder(search_bar& s) {
    apply_placeholder(s.placeholder.get());
    s.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

} // namespace mpapp

#endif // _WIN32
