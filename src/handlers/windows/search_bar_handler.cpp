// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_search_bar handler implementation.

#include "mpapp/handlers/windows/search_bar_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "winrt_strings.hpp"

namespace mpapp::internal {

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

void search_bar_handler<platform::windows>::map_text(basic_search_bar& s) {
    apply_text(s.text.get());
    s.text.changed.subscribe(text_slot_, text_cb_);
}
void search_bar_handler<platform::windows>::map_placeholder(basic_search_bar& s) {
    apply_placeholder(s.placeholder.get());
    s.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_search_bar so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_search_bar.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_search_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_search_bar*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_search_bar); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
