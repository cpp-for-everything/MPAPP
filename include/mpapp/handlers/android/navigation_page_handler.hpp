// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_navigation_page handler.
//
// Wraps a vertical LinearLayout with:
//   - row 0: a horizontal LinearLayout bar (Button "<" + TextView title)
//   - row 1: a FrameLayout content host swapped on each page_did_appear.

#ifndef MPAPP_HANDLERS_ANDROID_NAVIGATION_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_NAVIGATION_PAGE_HANDLER_HPP

#include <cstddef>
#include <string>

#include "../../internal/basic_navigation_page.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class navigation_page_handler<platform::android> {
public:
    navigation_page_handler();
    ~navigation_page_handler();

    navigation_page_handler(const navigation_page_handler&)            = delete;
    navigation_page_handler& operator=(const navigation_page_handler&) = delete;
    navigation_page_handler(navigation_page_handler&&)                 = delete;
    navigation_page_handler& operator=(navigation_page_handler&&)      = delete;

    void map_stack(basic_navigation_page& np);

    jobject native() const noexcept { return native_; }

private:
    void apply_top(view* new_top);
    void apply_title(const std::string& v);
    void apply_back_visibility(std::size_t depth);

    struct did_appear_cb {
        navigation_page_handler<platform::android>* self;
        void operator()(view* v) const { self->apply_top(v); }
    };
    struct depth_cb {
        navigation_page_handler<platform::android>* self;
        void operator()(std::size_t d) const { self->apply_back_visibility(d); }
    };

    jobject native_       = nullptr;   // LinearLayout (vertical)
    jobject bar_          = nullptr;   // LinearLayout (horizontal)
    jobject back_button_  = nullptr;   // Button
    jobject title_view_   = nullptr;   // TextView
    jobject content_host_ = nullptr;   // FrameLayout

    basic_navigation_page* bound_ = nullptr;

    did_appear_cb                   did_appear_cb_{this};
    depth_cb                        depth_cb_{this};
    signal_slot<view*>              did_appear_slot_{};
    signal_slot<const std::size_t&> depth_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_NAVIGATION_PAGE_HANDLER_HPP
