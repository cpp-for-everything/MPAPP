// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_title_bar handler implementation.

#include "mpapp/handlers/windows/title_bar_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

title_bar_handler<platform::windows>::title_bar_handler() {
    // mux::Controls::TitleBar requires WinUI 3 1.5+. The default-ctor
    // throws hresult_class_not_registered on hosts that haven't shipped
    // the Windows App SDK runtime; swallow that here so the rest of the
    // app still hosts.
    try {
        native_ = muxc::TitleBar{};
    } catch (...) {
        native_ = nullptr;
    }
}

title_bar_handler<platform::windows>::~title_bar_handler() = default;

void title_bar_handler<platform::windows>::apply_title(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        native_.Title(detail::to_hstring_utf8(v));
    } catch (...) { /* ignore */ }
}

void title_bar_handler<platform::windows>::apply_subtitle(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        native_.Subtitle(detail::to_hstring_utf8(v));
    } catch (...) { /* ignore */ }
}

void title_bar_handler<platform::windows>::map_title(basic_title_bar& t) {
    apply_title(t.title.get());
    t.title.changed.subscribe(title_slot_, title_cb_);
}
void title_bar_handler<platform::windows>::map_subtitle(basic_title_bar& t) {
    apply_subtitle(t.subtitle.get());
    t.subtitle.changed.subscribe(subtitle_slot_, subtitle_cb_);
}

} // namespace mpapp::internal
// --- ADR-0013 self-registration --------------------------------------------

namespace {

::winrt::Microsoft::UI::Xaml::UIElement
dispatch_title_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_title_bar*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_title_bar);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
