// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock handler for `mpapp::flyout_page`.

#ifndef MPAPP_HANDLERS_MOCK_FLYOUT_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_FLYOUT_PAGE_HANDLER_HPP

#include "../../flyout_page.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class flyout_page_handler<platform::mock> : public mock_handler_base {
public:
    flyout_page_handler() = default;
    ~flyout_page_handler() = default;

    flyout_page_handler(const flyout_page_handler&)            = delete;
    flyout_page_handler& operator=(const flyout_page_handler&) = delete;
    flyout_page_handler(flyout_page_handler&&)                 = delete;
    flyout_page_handler& operator=(flyout_page_handler&&)      = delete;

    void map_flyout(flyout_page& fp) {
        record_change("flyout.present", fp.flyout.get() != nullptr);
        fp.flyout.changed.subscribe(slot_flyout_, flyout_cb_);
    }

    void map_detail(flyout_page& fp) {
        record_change("detail.present", fp.detail.get() != nullptr);
        fp.detail.changed.subscribe(slot_detail_, detail_cb_);
    }

    void map_is_presented(flyout_page& fp) {
        record_change("is_presented", fp.is_presented.get());
        fp.is_presented.changed.subscribe(slot_pres_, pres_cb_);
    }

private:
    using self_t = flyout_page_handler<platform::mock>;

    struct slot_recorder {
        self_t*     self = nullptr;
        const char* tag  = "";
        void operator()(page* p) const { self->record_change(tag, p != nullptr); }
    };
    struct pres_recorder {
        self_t* self = nullptr;
        void operator()(bool v) const { self->record_change("is_presented", v); }
    };

    slot_recorder flyout_cb_{this, "flyout.present"};
    slot_recorder detail_cb_{this, "detail.present"};
    pres_recorder pres_cb_{this};

    signal_slot<page* const&> slot_flyout_{};
    signal_slot<page* const&> slot_detail_{};
    signal_slot<const bool&>  slot_pres_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_FLYOUT_PAGE_HANDLER_HPP
