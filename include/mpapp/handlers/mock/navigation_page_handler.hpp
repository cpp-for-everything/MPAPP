// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock handler for `mpapp::basic_navigation_page`.
//
// Records lifecycle events from the page_stack engine so tests can
// assert: "when I push, did page_will_appear fire for the right view?"
// and similar. The handler doesn't drive native widgets — that's the
// per-platform real handler's job.

#ifndef MPAPP_HANDLERS_MOCK_NAVIGATION_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_NAVIGATION_PAGE_HANDLER_HPP

#include <cstddef>

#include "../../internal/basic_navigation_page.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class navigation_page_handler<platform::mock> : public mock_handler_base {
public:
    navigation_page_handler() = default;
    ~navigation_page_handler() = default;

    navigation_page_handler(const navigation_page_handler&)            = delete;
    navigation_page_handler& operator=(const navigation_page_handler&) = delete;
    navigation_page_handler(navigation_page_handler&&)                 = delete;
    navigation_page_handler& operator=(navigation_page_handler&&)      = delete;

    void map_stack(basic_navigation_page& np) {
        record_change("stack.depth", np.stack().depth());
        // Subscribe to the four lifecycle signals.
        np.stack().page_will_appear.subscribe(slot_will_app_, will_app_);
        np.stack().page_did_appear.subscribe(slot_did_app_, did_app_);
        np.stack().page_will_disappear.subscribe(slot_will_dis_, will_dis_);
        np.stack().page_did_disappear.subscribe(slot_did_dis_, did_dis_);
        np_ = &np;
    }

    void map_current_page(basic_navigation_page& np) {
        record_change("current_page.present", np.current_page.get() != nullptr);
        np.current_page.changed.subscribe(slot_cur_, cur_cb_);
        np_ = &np;
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_navigation_page& /*x*/) noexcept {}


private:
    using self_t = navigation_page_handler<platform::mock>;

    struct lifecycle_recorder {
        self_t*     self = nullptr;
        const char* tag  = "";
        void operator()(view* v) const { self->record_change(tag, v != nullptr); }
    };

    struct cur_recorder {
        self_t* self = nullptr;
        void operator()(basic_page* p) const { self->record_change("current_page.present", p != nullptr); }
    };

    basic_navigation_page* np_ = nullptr;

    lifecycle_recorder will_app_{this, "page_will_appear"};
    lifecycle_recorder did_app_{this,  "page_did_appear"};
    lifecycle_recorder will_dis_{this, "page_will_disappear"};
    lifecycle_recorder did_dis_{this,  "page_did_disappear"};
    cur_recorder       cur_cb_{this};

    signal_slot<view*> slot_will_app_{};
    signal_slot<view*> slot_did_app_{};
    signal_slot<view*> slot_will_dis_{};
    signal_slot<view*> slot_did_dis_{};
    signal_slot<basic_page* const&> slot_cur_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_NAVIGATION_PAGE_HANDLER_HPP
