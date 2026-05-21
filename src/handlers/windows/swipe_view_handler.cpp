// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 swipe_view handler implementation.

#include "mpapp/handlers/windows/swipe_view_handler.hpp"

#if defined(_WIN32)

#include <vector>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/swipe_item_menu_item.hpp"

#include "winrt_strings.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

swipe_view_handler<platform::windows>::swipe_view_handler() {
    // `muxc::SwipeControl` requires WinAppSDK runtime to be present.
    // Wrap construction in try/catch so a missing-runtime host gets a
    // graceful nullptr handle instead of a crash on launch — matches the
    // `title_bar_handler` shape for the same reason.
    try {
        native_ = muxc::SwipeControl{};
    } catch (...) {
        native_ = nullptr;
    }
}

swipe_view_handler<platform::windows>::~swipe_view_handler() = default;

void swipe_view_handler<platform::windows>::apply_content(view* v) {
    if (native_ == nullptr) return;
    if (v == nullptr) {
        try { native_.Content(nullptr); } catch (...) {}
        return;
    }
    // ADR-0013 registry first; if no widget is registered for the child
    // type, leave the content empty.
    if (auto el = detail::windows_dispatch::dispatch(v); el != nullptr) {
        try { native_.Content(el); } catch (...) {}
    } else {
        try { native_.Content(nullptr); } catch (...) {}
    }
}

namespace {

// Build a `muxc::SwipeItems` from the cross-platform vector. Only
// `swipe_item_menu_item` entries map cleanly onto WinUI's `SwipeItem`
// (icon + text). Custom-content entries (`swipe_item_view`) are silently
// skipped — the SwipeControl pane will only show the named-action subset
// in this M-04b landing. The richer custom-content branch lands alongside
// gesture-event plumbing in a follow-up batch.
muxc::SwipeItems build_swipe_items(const std::vector<view*>& items) {
    muxc::SwipeItems out{};
    try {
        out = muxc::SwipeItems{};
    } catch (...) {
        return muxc::SwipeItems{nullptr};
    }
    for (view* v : items) {
        auto* m = dynamic_cast<swipe_item_menu_item*>(v);
        if (m == nullptr) continue;
        try {
            muxc::SwipeItem si{};
            si.Text(detail::to_hstring_utf8(m->text.get()));
            const std::string& uri = m->icon_uri.get();
            if (!uri.empty()) {
                muxc::SymbolIconSource sym{};
                // Default to a tag-style symbol — a richer URI→IconSource
                // resolver lands with image-source plumbing.
                sym.Symbol(::winrt::Microsoft::UI::Xaml::Controls::Symbol::Tag);
                si.IconSource(sym);
            }
            out.Append(si);
        } catch (...) {
            // Skip malformed entries silently.
        }
    }
    return out;
}

} // namespace

void swipe_view_handler<platform::windows>::apply_left_items(const std::vector<view*>& items) {
    if (native_ == nullptr) return;
    try {
        native_.LeftItems(build_swipe_items(items));
    } catch (...) {}
}

void swipe_view_handler<platform::windows>::apply_right_items(const std::vector<view*>& items) {
    if (native_ == nullptr) return;
    try {
        native_.RightItems(build_swipe_items(items));
    } catch (...) {}
}

void swipe_view_handler<platform::windows>::map_content(swipe_view& sv) {
    apply_content(sv.content.get());
    sv.content.changed.subscribe(content_slot_, content_cb_);
}

void swipe_view_handler<platform::windows>::map_left_items(swipe_view& sv) {
    apply_left_items(sv.left_items.get());
    sv.left_items.changed.subscribe(left_slot_, left_cb_);
}

void swipe_view_handler<platform::windows>::map_right_items(swipe_view& sv) {
    apply_right_items(sv.right_items.get());
    sv.right_items.changed.subscribe(right_slot_, right_cb_);
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_swipe_view(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::swipe_view*>(v); s && s->has_handler()) {
        return s->handler().native();
    }
    return nullptr;
}

struct swipe_view_registrar {
    swipe_view_registrar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_swipe_view);
    }
};

[[maybe_unused]] swipe_view_registrar _swipe_view_reg;

} // namespace

#endif // _WIN32
