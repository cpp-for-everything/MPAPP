// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0003 — WinUI 3 button spike.
//
// Implementation of `button_handler<platform::windows>` against C++/WinRT
// WinUI 3. The handler:
//   1. Constructs a native `Microsoft.UI.Xaml.Controls.Button`.
//   2. `map_text` pushes the current `text` value into `Content` and
//      subscribes a slot so future `text.set(...)` calls propagate.
//   3. `map_clicked` registers a native `Click` handler that fires the
//      cross-platform `clicked` signal.
//
// Threading: WinUI controls must be touched on the UI thread. The host
// (examples/windows_button_spike/main.cpp) calls these methods after the
// Application has started, so we're already on the right apartment.

#include "mpapp/handlers/windows/button_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
// IButtonBase::Click lives in the Primitives header — including
// Controls.h alone leaves `consume_*::Click` forward-declared only.
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "winrt_strings.hpp"

namespace mpapp {

void button_handler<platform::windows>::apply_text(std::string_view text) {
    if (native_ != nullptr) {
        native_.Content(winrt::box_value(detail::to_hstring_utf8(text)));
    }
}

button_handler<platform::windows>::button_handler() {
    native_ = winrt::Microsoft::UI::Xaml::Controls::Button{};
}

button_handler<platform::windows>::~button_handler() {
    // Unregister the click handler explicitly so the WinRT runtime doesn't
    // invoke a destroyed lambda. Zero-token means "never wired".
    if (native_ != nullptr && click_token_.value != 0) {
        try {
            native_.Click(click_token_);
        } catch (...) {
            // Destructors must not propagate across the interop edge.
        }
        click_token_ = {};
    }
}

void button_handler<platform::windows>::map_text(button& b) {
    apply_text(b.text.get());
    // signal::subscribe calls disconnect first, so this is idempotent.
    b.text.changed.subscribe(text_slot_, text_callback_);
}

void button_handler<platform::windows>::map_clicked(button& b) {
    if (native_ == nullptr) {
        return;
    }
    if (click_token_.value != 0) {
        native_.Click(click_token_);
        click_token_ = {};
    }
    // `b`'s address is stable for the handler's lifetime.
    button* target = &b;
    click_token_ = native_.Click([target](
        winrt::Windows::Foundation::IInspectable const&,
        winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
        target->clicked.emit();
    });
}

} // namespace mpapp

#endif // _WIN32
