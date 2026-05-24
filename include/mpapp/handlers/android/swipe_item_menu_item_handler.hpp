// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_swipe_item_menu_item handler.
//
// `android.widget.Button` rendering the action's text basic_label. Icon URI
// plumbing is symbolic for M-04b — the value is captured on the
// Observable but no setCompoundDrawables call is made yet (the broader
// basic_image-source resolver lands separately). The `invoked` signal is
// declared but the OnClickListener wiring is deferred until the
// shared MppClickRouter pattern is generalised beyond `basic_button` — for
// M-04b, programmatic `invoked.emit()` on the C++ side is the only path
// (acceptable per the worker prompt's degradation contract).

#ifndef MPAPP_HANDLERS_ANDROID_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_swipe_item_menu_item.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class swipe_item_menu_item_handler<platform::android> {
public:
    swipe_item_menu_item_handler();
    ~swipe_item_menu_item_handler();

    swipe_item_menu_item_handler(const swipe_item_menu_item_handler&)            = delete;
    swipe_item_menu_item_handler& operator=(const swipe_item_menu_item_handler&) = delete;
    swipe_item_menu_item_handler(swipe_item_menu_item_handler&&)                 = delete;
    swipe_item_menu_item_handler& operator=(swipe_item_menu_item_handler&&)      = delete;

    void map_text(basic_swipe_item_menu_item& m);
    void map_icon_uri(basic_swipe_item_menu_item& m);
    void map_invoked(basic_swipe_item_menu_item& m);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_icon_uri(const std::string& v);

    struct text_cb_t {
        swipe_item_menu_item_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct icon_cb_t {
        swipe_item_menu_item_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_icon_uri(v); }
    };

    jobject native_ = nullptr; // android.widget.Button (global ref)

    text_cb_t                       text_cb_{this};
    icon_cb_t                       icon_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const std::string&> icon_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP
