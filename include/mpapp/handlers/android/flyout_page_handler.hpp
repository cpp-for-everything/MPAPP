// SPDX-License-Identifier: Apache-2.0
// Android flyout_page handler. A horizontal LinearLayout with the
// flyout FrameLayout on the left and detail FrameLayout on the right.
// is_presented toggles the flyout container's visibility.

#ifndef MPAPP_HANDLERS_ANDROID_FLYOUT_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_FLYOUT_PAGE_HANDLER_HPP

#include "../../flyout_page.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class flyout_page_handler<platform::android> {
public:
    flyout_page_handler();
    ~flyout_page_handler();

    flyout_page_handler(const flyout_page_handler&)            = delete;
    flyout_page_handler& operator=(const flyout_page_handler&) = delete;
    flyout_page_handler(flyout_page_handler&&)                 = delete;
    flyout_page_handler& operator=(flyout_page_handler&&)      = delete;

    void map_flyout(flyout_page& fp);
    void map_detail(flyout_page& fp);
    void map_is_presented(flyout_page& fp);

    jobject native() const noexcept { return native_; }

private:
    void apply_flyout(page* p);
    void apply_detail(page* p);
    void apply_is_presented(bool v);

    struct flyout_cb_t {
        flyout_page_handler<platform::android>* self;
        void operator()(page* p) const { self->apply_flyout(p); }
    };
    struct detail_cb_t {
        flyout_page_handler<platform::android>* self;
        void operator()(page* p) const { self->apply_detail(p); }
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
    signal_slot<page* const&> flyout_slot_{};
    signal_slot<page* const&> detail_slot_{};
    signal_slot<const bool&>  presented_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_FLYOUT_PAGE_HANDLER_HPP
