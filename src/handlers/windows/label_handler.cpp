// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0003 — WinUI 3 basic_button spike.
//
// Implementation of `label_handler<platform::windows>` against C++/WinRT
// WinUI 3 `TextBlock`.

#include "mpapp/handlers/windows/label_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "winrt_strings.hpp"

namespace mpapp::internal {

void label_handler<platform::windows>::apply_text(std::string_view text) {
    if (native_ != nullptr) {
        native_.Text(detail::to_hstring_utf8(text));
    }
}

label_handler<platform::windows>::label_handler() {
    native_ = winrt::Microsoft::UI::Xaml::Controls::TextBlock{};
}

label_handler<platform::windows>::~label_handler() = default;

void label_handler<platform::windows>::map_text(basic_label& l) {
    apply_text(l.text.get());
    l.text.changed.subscribe(text_slot_, text_callback_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_label.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_label(::mpapp::view* v) {
    if (auto* l = dynamic_cast<::mpapp::internal::basic_label*>(v); l && l->has_handler()) {
        return l->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_label); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
