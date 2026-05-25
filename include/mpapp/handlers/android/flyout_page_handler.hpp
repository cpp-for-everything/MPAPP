// SPDX-License-Identifier: Apache-2.0
// Android basic_flyout_page handler. A horizontal LinearLayout with the
// flyout FrameLayout on the left and detail FrameLayout on the right.
// is_presented toggles the flyout container's visibility.

#ifndef MPAPP_HANDLERS_ANDROID_FLYOUT_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_FLYOUT_PAGE_HANDLER_HPP

#include "../../internal/basic_flyout_page.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class flyout_page_handler<platform::android> {
public:
    flyout_page_handler();
    ~flyout_page_handler();

    flyout_page_handler(const flyout_page_handler&)            = delete;
    flyout_page_handler& operator=(const flyout_page_handler&) = delete;
    flyout_page_handler(flyout_page_handler&&)                 = delete;
    flyout_page_handler& operator=(flyout_page_handler&&)      = delete;

    void map_flyout(basic_flyout_page& fp);
    void map_detail(basic_flyout_page& fp);
    void map_is_presented(basic_flyout_page& fp);

    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_flyout_page& /*x*/) noexcept {}


private:
    void apply_flyout(basic_page* p);
    void apply_detail(basic_page* p);
    void apply_is_presented(bool v);

    struct flyout_cb_t {
        flyout_page_handler<platform::android>* self;
        void operator()(basic_page* p) const { self->apply_flyout(p); }
    };
    struct detail_cb_t {
        flyout_page_handler<platform::android>* self;
        void operator()(basic_page* p) const { self->apply_detail(p); }
    };
    struct presented_cb_t {
        flyout_page_handler<platform::android>* self;
        void operator()(bool v) const { self->apply_is_presented(v); }
    };

    jobject native_      = nullptr;   // LinearLayout horizontal
    jobject flyout_host_ = nullptr;   // FrameLayout (left)
    jobject detail_host_ = nullptr;   // FrameLayout (right)

    flyout_cb_t    flyout_cb_{this};
    detail_cb_t    detail_cb_{this};
    presented_cb_t presented_cb_{this};
    signal_slot<basic_page* const&> flyout_slot_{};
    signal_slot<basic_page* const&> detail_slot_{};
    signal_slot<const bool&>  presented_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_FLYOUT_PAGE_HANDLER_HPP
