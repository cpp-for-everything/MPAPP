// SPDX-License-Identifier: Apache-2.0
// WinUI 3 content_view handler implementation.

#include "mpapp/handlers/windows/content_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/box_view.hpp"
#include "mpapp/button.hpp"
#include "mpapp/check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/entry.hpp"
#include "mpapp/handlers/windows/box_view_handler.hpp"
#include "mpapp/handlers/windows/button_handler.hpp"
#include "mpapp/handlers/windows/check_box_handler.hpp"
#include "mpapp/handlers/windows/editor_handler.hpp"
#include "mpapp/handlers/windows/entry_handler.hpp"
#include "mpapp/handlers/windows/label_handler.hpp"
#include "mpapp/handlers/windows/radio_button_handler.hpp"
#include "mpapp/handlers/windows/slider_handler.hpp"
#include "mpapp/handlers/windows/stack_layout_handler.hpp"
#include "mpapp/handlers/windows/switch_handler.hpp"
#include "mpapp/label.hpp"
#include "mpapp/radio_button.hpp"
#include "mpapp/slider.hpp"
#include "mpapp/stack_layout.hpp"
#include "mpapp/switch_.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

content_view_handler<platform::windows>::content_view_handler() {
    native_ = muxc::ContentControl{};
}

content_view_handler<platform::windows>::~content_view_handler() = default;

void content_view_handler<platform::windows>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    view* raw = v.get();
    if (raw == nullptr) { native_.Content(nullptr); return; }
    if (auto* sl = dynamic_cast<stack_layout*>(raw); sl && sl->has_handler()) { native_.Content(sl->handler().native()); return; }
    if (auto* b  = dynamic_cast<button*>(raw);       b  && b->has_handler())  { native_.Content(b->handler().native());  return; }
    if (auto* l  = dynamic_cast<label*>(raw);        l  && l->has_handler())  { native_.Content(l->handler().native());  return; }
    if (auto* e  = dynamic_cast<entry*>(raw);        e  && e->has_handler())  { native_.Content(e->handler().native());  return; }
    if (auto* sw = dynamic_cast<switch_*>(raw);      sw && sw->has_handler()) { native_.Content(sw->handler().native()); return; }
    if (auto* cb = dynamic_cast<check_box*>(raw);    cb && cb->has_handler()) { native_.Content(cb->handler().native()); return; }
    if (auto* rb = dynamic_cast<radio_button*>(raw); rb && rb->has_handler()) { native_.Content(rb->handler().native()); return; }
    if (auto* s2 = dynamic_cast<slider*>(raw);       s2 && s2->has_handler()) { native_.Content(s2->handler().native()); return; }
    if (auto* ed = dynamic_cast<editor*>(raw);       ed && ed->has_handler()) { native_.Content(ed->handler().native()); return; }
    if (auto* bx = dynamic_cast<box_view*>(raw);     bx && bx->has_handler()) { native_.Content(bx->handler().native()); return; }
}

void content_view_handler<platform::windows>::map_content(content_view& c) {
    apply_content(c.content.get());
    c.content.changed.subscribe(content_slot_, content_cb_);
}

void content_view_handler<platform::windows>::bind_content(content_view& c, view& child) {
    c.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

#endif // _WIN32
