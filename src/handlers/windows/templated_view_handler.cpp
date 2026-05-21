// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 templated_view handler implementation.

#include "mpapp/handlers/windows/templated_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "mpapp/templated_view.hpp"
#include "mpapp/view.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

templated_view_handler<platform::windows>::templated_view_handler() {
    native_ = muxc::ContentControl{};
}

templated_view_handler<platform::windows>::~templated_view_handler() = default;

void templated_view_handler<platform::windows>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    view* raw = v.get();
    if (raw == nullptr) { native_.Content(nullptr); return; }
    // ADR-0013 registry — every child widget type that registers itself
    // resolves here. Until all widgets are migrated to self-register,
    // children of legacy non-registered types render as empty.
    if (auto el = detail::windows_dispatch::dispatch(raw); el != nullptr) {
        native_.Content(el);
    } else {
        native_.Content(nullptr);
    }
}

void templated_view_handler<platform::windows>::apply_template_id(const std::string& v) {
    // P3 templating engine is deferred — record the id so external
    // tooling (XAML compiler, hot-reload daemon) can verify the wiring
    // end-to-end. No native side effect today.
    template_id_ = v;
}

void templated_view_handler<platform::windows>::map_content(templated_view& t) {
    apply_content(t.content.get());
    t.content.changed.subscribe(content_slot_, content_cb_);
}

void templated_view_handler<platform::windows>::map_template_id(templated_view& t) {
    apply_template_id(t.template_id.get());
    t.template_id.changed.subscribe(template_id_slot_, template_id_cb_);
}

void templated_view_handler<platform::windows>::bind_content(templated_view& t, view& child) {
    t.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_templated_view(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::templated_view*>(v); t && t->has_handler()) {
        return t->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_templated_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
