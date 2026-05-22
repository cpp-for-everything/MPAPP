// SPDX-License-Identifier: Apache-2.0
// WinUI 3 switch_cell handler implementation.

#include "mpapp/handlers/windows/switch_cell_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

switch_cell_handler<platform::windows>::switch_cell_handler() {
    native_     = muxc::Border{};
    grid_       = muxc::Grid{};
    text_block_ = muxc::TextBlock{};
    toggle_     = muxc::ToggleSwitch{};

    // Two-column layout: label (star) + toggle (auto).
    muxc::ColumnDefinition col_text{};
    col_text.Width(mux::GridLengthHelper::FromValueAndType(1.0, mux::GridUnitType::Star));
    muxc::ColumnDefinition col_toggle{};
    col_toggle.Width(mux::GridLengthHelper::FromValueAndType(1.0, mux::GridUnitType::Auto));
    grid_.ColumnDefinitions().Append(col_text);
    grid_.ColumnDefinitions().Append(col_toggle);

    text_block_.VerticalAlignment(mux::VerticalAlignment::Center);
    muxc::Grid::SetColumn(text_block_, 0);
    muxc::Grid::SetColumn(toggle_, 1);

    // ToggleSwitch defaults render with a leading on/off label and large
    // gutter; squeeze for a cell row.
    toggle_.OnContent(nullptr);
    toggle_.OffContent(nullptr);

    grid_.Children().Append(text_block_);
    grid_.Children().Append(toggle_);

    native_.Padding({12.0, 6.0, 12.0, 6.0});
    native_.Child(grid_);
}

switch_cell_handler<platform::windows>::~switch_cell_handler() {
    if (toggle_ != nullptr && toggled_token_.value != 0) {
        try { toggle_.Toggled(toggled_token_); } catch (...) {}
        toggled_token_ = {};
    }
}

void switch_cell_handler<platform::windows>::apply_text(const std::string& v) {
    if (text_block_ == nullptr) return;
    text_block_.Text(detail::to_hstring_utf8(v));
}

void switch_cell_handler<platform::windows>::apply_on(bool v) {
    if (toggle_ == nullptr) return;
    suppress_echo_ = true;
    toggle_.IsOn(v);
    suppress_echo_ = false;
}

void switch_cell_handler<platform::windows>::map_text(switch_cell& c) {
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);
}

void switch_cell_handler<platform::windows>::map_on(switch_cell& c) {
    bound_ = &c;
    apply_on(c.on.get());
    c.on.changed.subscribe(on_slot_, on_cb_);

    if (toggle_ == nullptr) return;
    if (toggled_token_.value != 0) {
        toggle_.Toggled(toggled_token_);
        toggled_token_ = {};
    }
    switch_cell* target = &c;
    auto* self = this;
    toggled_token_ = toggle_.Toggled(
        [target, self](winrt::Windows::Foundation::IInspectable const& sender,
                       mux::RoutedEventArgs const&) {
            if (self->suppress_echo_) return;
            auto ts = sender.as<muxc::ToggleSwitch>();
            const bool v = ts.IsOn();
            if (target->on.get() != v) {
                target->on.set(v);
            }
            target->on_changed.emit(v);
        });
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_switch_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::switch_cell*>(v); c && c->has_sc_handler()) {
        return c->sc_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_switch_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
