// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 editor handler implementation.

#include "mpapp/handlers/windows/editor_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "winrt_strings.hpp"

namespace mpapp {

editor_handler<platform::windows>::editor_handler() {
    native_ = winrt::Microsoft::UI::Xaml::Controls::TextBox{};
    native_.AcceptsReturn(true);
    native_.TextWrapping(winrt::Microsoft::UI::Xaml::TextWrapping::Wrap);
}

editor_handler<platform::windows>::~editor_handler() {
    if (native_ != nullptr && text_changed_token_.value != 0) {
        try { native_.TextChanged(text_changed_token_); } catch (...) {}
        text_changed_token_ = {};
    }
}

void editor_handler<platform::windows>::apply_text(std::string_view text) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    native_.Text(detail::to_hstring_utf8(text));
    suppress_echo_ = false;
}
void editor_handler<platform::windows>::apply_placeholder(std::string_view text) {
    if (native_ != nullptr) native_.PlaceholderText(detail::to_hstring_utf8(text));
}
void editor_handler<platform::windows>::apply_is_read_only(bool ro) {
    if (native_ != nullptr) native_.IsReadOnly(ro);
}

void editor_handler<platform::windows>::map_text(editor& e) {
    bound_ = &e;
    apply_text(e.text.get());
    e.text.changed.subscribe(text_slot_, text_cb_);

    if (native_ == nullptr) return;
    if (text_changed_token_.value != 0) {
        native_.TextChanged(text_changed_token_);
        text_changed_token_ = {};
    }
    editor* target = &e;
    auto* self = this;
    text_changed_token_ = native_.TextChanged(
        [target, self](winrt::Windows::Foundation::IInspectable const& sender,
                       winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const&) {
            if (self->suppress_echo_) return;
            auto tb = sender.as<winrt::Microsoft::UI::Xaml::Controls::TextBox>();
            const std::wstring wide{tb.Text()};
            const std::string utf8 = detail::wstring_to_utf8(wide);
            if (target->text.get() != utf8) {
                target->text.set(utf8);
            }
        });
}

void editor_handler<platform::windows>::map_placeholder(editor& e) {
    apply_placeholder(e.placeholder.get());
    e.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}
void editor_handler<platform::windows>::map_is_read_only(editor& e) {
    apply_is_read_only(e.is_read_only.get());
    e.is_read_only.changed.subscribe(readonly_slot_, readonly_cb_);
}

} // namespace mpapp

#endif // _WIN32
