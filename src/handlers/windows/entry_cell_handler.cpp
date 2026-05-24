// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_entry_cell handler implementation.

#include "mpapp/handlers/windows/entry_cell_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace mux   = ::winrt::Microsoft::UI::Xaml;
namespace muxc  = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxi  = ::winrt::Microsoft::UI::Xaml::Input;
namespace ws    = ::winrt::Windows::System;

namespace {

muxi::InputScopeNameValue keyboard_to_scope(keyboard_kind k) {
    using V = muxi::InputScopeNameValue;
    switch (k) {
        case keyboard_kind::email:     return V::EmailSmtpAddress;
        case keyboard_kind::numeric:   return V::Number;
        case keyboard_kind::telephone: return V::TelephoneNumber;
        case keyboard_kind::url:       return V::Url;
        case keyboard_kind::chat:      return V::Chat;
        case keyboard_kind::text:
        case keyboard_kind::default_:
        default:                       return V::Default;
    }
}

} // namespace

entry_cell_handler<platform::windows>::entry_cell_handler() {
    native_      = muxc::Border{};
    grid_        = muxc::Grid{};
    label_block_ = muxc::TextBlock{};
    text_box_    = muxc::TextBox{};

    // 2-col layout: basic_label (auto) + text box (star).
    muxc::ColumnDefinition col_label{};
    col_label.Width(mux::GridLengthHelper::FromValueAndType(1.0, mux::GridUnitType::Auto));
    muxc::ColumnDefinition col_text{};
    col_text.Width(mux::GridLengthHelper::FromValueAndType(1.0, mux::GridUnitType::Star));
    grid_.ColumnDefinitions().Append(col_label);
    grid_.ColumnDefinitions().Append(col_text);

    label_block_.VerticalAlignment(mux::VerticalAlignment::Center);
    label_block_.Margin({0.0, 0.0, 12.0, 0.0});
    muxc::Grid::SetColumn(label_block_, 0);
    muxc::Grid::SetColumn(text_box_, 1);

    grid_.Children().Append(label_block_);
    grid_.Children().Append(text_box_);

    native_.Padding({12.0, 6.0, 12.0, 6.0});
    native_.Child(grid_);
}

entry_cell_handler<platform::windows>::~entry_cell_handler() {
    if (text_box_ != nullptr) {
        if (text_changed_token_.value != 0) {
            try { text_box_.TextChanged(text_changed_token_); } catch (...) {}
            text_changed_token_ = {};
        }
        if (key_down_token_.value != 0) {
            try { text_box_.KeyDown(key_down_token_); } catch (...) {}
            key_down_token_ = {};
        }
    }
}

void entry_cell_handler<platform::windows>::apply_label(const std::string& v) {
    if (label_block_ == nullptr) return;
    label_block_.Text(detail::to_hstring_utf8(v));
}

void entry_cell_handler<platform::windows>::apply_text(const std::string& v) {
    if (text_box_ == nullptr) return;
    suppress_echo_ = true;
    text_box_.Text(detail::to_hstring_utf8(v));
    suppress_echo_ = false;
}

void entry_cell_handler<platform::windows>::apply_placeholder(const std::string& v) {
    if (text_box_ == nullptr) return;
    text_box_.PlaceholderText(detail::to_hstring_utf8(v));
}

void entry_cell_handler<platform::windows>::apply_keyboard(keyboard_kind k) {
    if (text_box_ == nullptr) return;
    muxi::InputScope scope{};
    muxi::InputScopeName name{};
    name.NameValue(keyboard_to_scope(k));
    scope.Names().Append(name);
    text_box_.InputScope(scope);
}

void entry_cell_handler<platform::windows>::map_label(basic_entry_cell& c) {
    apply_label(c.label.get());
    c.label.changed.subscribe(label_slot_, label_cb_);
}

void entry_cell_handler<platform::windows>::map_text(basic_entry_cell& c) {
    bound_ = &c;
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);

    if (text_box_ == nullptr) return;
    if (text_changed_token_.value != 0) {
        text_box_.TextChanged(text_changed_token_);
        text_changed_token_ = {};
    }
    basic_entry_cell* target = &c;
    auto* self = this;
    text_changed_token_ = text_box_.TextChanged(
        [target, self](winrt::Windows::Foundation::IInspectable const& sender,
                       muxc::TextChangedEventArgs const&) {
            if (self->suppress_echo_) return;
            auto tb = sender.as<muxc::TextBox>();
            const std::wstring wide{tb.Text()};
            const std::string utf8 = detail::wstring_to_utf8(wide);
            if (target->text.get() != utf8) {
                target->text.set(utf8);
            }
        });

    if (key_down_token_.value != 0) {
        text_box_.KeyDown(key_down_token_);
        key_down_token_ = {};
    }
    key_down_token_ = text_box_.KeyDown(
        [target](winrt::Windows::Foundation::IInspectable const&,
                 muxi::KeyRoutedEventArgs const& args) {
            if (args.Key() == ws::VirtualKey::Enter) {
                target->completed.emit(target->text.get());
            }
        });
}

void entry_cell_handler<platform::windows>::map_placeholder(basic_entry_cell& c) {
    apply_placeholder(c.placeholder.get());
    c.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

void entry_cell_handler<platform::windows>::map_keyboard(basic_entry_cell& c) {
    apply_keyboard(c.keyboard.get());
    c.keyboard.changed.subscribe(keyboard_slot_, keyboard_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_entry_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_entry_cell*>(v); c && c->has_ec_handler()) {
        return c->ec_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_entry_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
