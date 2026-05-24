// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 follow-up — WinUI 3 basic_entry handler implementation.

#include "mpapp/handlers/windows/entry_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "winrt_strings.hpp"

namespace mpapp::internal {

entry_handler<platform::windows>::entry_handler() {
    native_ = winrt::Microsoft::UI::Xaml::Controls::TextBox{};
}

entry_handler<platform::windows>::~entry_handler() {
    if (native_ != nullptr && text_changed_token_.value != 0) {
        try {
            native_.TextChanged(text_changed_token_);
        } catch (...) {
            // Destructors must not propagate across the interop edge.
        }
        text_changed_token_ = {};
    }
}

void entry_handler<platform::windows>::apply_text(std::string_view text) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    native_.Text(detail::to_hstring_utf8(text));
    suppress_echo_ = false;
}

void entry_handler<platform::windows>::apply_placeholder(std::string_view text) {
    if (native_ != nullptr) {
        native_.PlaceholderText(detail::to_hstring_utf8(text));
    }
}

void entry_handler<platform::windows>::apply_is_read_only(bool ro) {
    if (native_ != nullptr) {
        native_.IsReadOnly(ro);
    }
}

void entry_handler<platform::windows>::map_text(basic_entry& e) {
    bound_ = &e;
    apply_text(e.text.get());
    e.text.changed.subscribe(text_slot_, text_cb_);

    if (native_ == nullptr) return;
    if (text_changed_token_.value != 0) {
        native_.TextChanged(text_changed_token_);
        text_changed_token_ = {};
    }
    basic_entry* target = &e;
    auto* self = this;
    text_changed_token_ = native_.TextChanged(
        [target, self](winrt::Windows::Foundation::IInspectable const& sender,
                       winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const&) {
            if (self->suppress_echo_) return;
            auto tb = sender.as<winrt::Microsoft::UI::Xaml::Controls::TextBox>();
            const auto native_text = tb.Text();
            // winrt::hstring → std::string (UTF-8). hstring is UTF-16
            // internally; widen via the conversion helper used elsewhere.
            const std::wstring wide{native_text};
            const std::string utf8 = detail::wstring_to_utf8(wide);
            if (target->text.get() != utf8) {
                target->text.set(utf8);
            }
        });
}

void entry_handler<platform::windows>::map_placeholder(basic_entry& e) {
    apply_placeholder(e.placeholder.get());
    e.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

void entry_handler<platform::windows>::map_is_read_only(basic_entry& e) {
    apply_is_read_only(e.is_read_only.get());
    e.is_read_only.changed.subscribe(readonly_slot_, readonly_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_entry so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_entry.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_entry(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_entry*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_entry); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
