// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_picker handler implementation.

#include "mpapp/handlers/windows/picker_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

picker_handler<platform::windows>::picker_handler() {
    native_ = muxc::ComboBox{};
}

picker_handler<platform::windows>::~picker_handler() = default;

void picker_handler<platform::windows>::apply_items(const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    try {
        auto items = native_.Items();
        items.Clear();
        for (const auto& s : v) {
            items.Append(::winrt::box_value(detail::to_hstring_utf8(s)));
        }
    } catch (...) {}
    suppress_echo_ = false;
}

void picker_handler<platform::windows>::apply_selected_index(int v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    try {
        native_.SelectedIndex(v);
    } catch (...) {}
    suppress_echo_ = false;
}

void picker_handler<platform::windows>::apply_title(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        native_.PlaceholderText(detail::to_hstring_utf8(v));
    } catch (...) {}
}

void picker_handler<platform::windows>::map_items(basic_picker& p) {
    apply_items(p.items.get());
    p.items.changed.subscribe(items_slot_, items_cb_);
}
void picker_handler<platform::windows>::map_selected_index(basic_picker& p) {
    apply_selected_index(p.selected_index.get());
    p.selected_index.changed.subscribe(selected_slot_, selected_cb_);
}
void picker_handler<platform::windows>::map_title(basic_picker& p) {
    apply_title(p.title.get());
    p.title.changed.subscribe(title_slot_, title_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_picker so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_picker.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_picker(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_picker*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_picker); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
