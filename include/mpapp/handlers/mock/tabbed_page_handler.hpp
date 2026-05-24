// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock handler for `mpapp::basic_tabbed_page`.

#ifndef MPAPP_HANDLERS_MOCK_TABBED_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TABBED_PAGE_HANDLER_HPP

#include <cstddef>

#include "../../platform.hpp"
#include "../../internal/basic_tabbed_page.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class tabbed_page_handler<platform::mock> : public mock_handler_base {
public:
    tabbed_page_handler() = default;
    ~tabbed_page_handler() = default;

    tabbed_page_handler(const tabbed_page_handler&)            = delete;
    tabbed_page_handler& operator=(const tabbed_page_handler&) = delete;
    tabbed_page_handler(tabbed_page_handler&&)                 = delete;
    tabbed_page_handler& operator=(tabbed_page_handler&&)      = delete;

    void map_children(basic_tabbed_page& tp) {
        record_change("children.count", tp.children.get().size());
        tp.children.changed.subscribe(slot_kids_, kids_cb_);
    }

    void map_selected_index(basic_tabbed_page& tp) {
        record_change("selected_index", tp.selected_index.get());
        tp.selected_index.changed.subscribe(slot_sel_, sel_cb_);
    }

    void map_lifecycle(basic_tabbed_page& tp) {
        tp.tab_will_appear.subscribe(slot_will_app_, lc_will_app_);
        tp.tab_did_appear.subscribe(slot_did_app_, lc_did_app_);
        tp.tab_will_disappear.subscribe(slot_will_dis_, lc_will_dis_);
        tp.tab_did_disappear.subscribe(slot_did_dis_, lc_did_dis_);
    }

private:
    using self_t = tabbed_page_handler<platform::mock>;

    struct kids_recorder {
        self_t* self = nullptr;
        void operator()(const std::vector<basic_page*>& v) const { self->record_change("children.count", v.size()); }
    };
    struct sel_recorder {
        self_t* self = nullptr;
        void operator()(int v) const { self->record_change("selected_index", v); }
    };
    struct lc_recorder {
        self_t*     self = nullptr;
        const char* tag  = "";
        void operator()(basic_page* p) const { self->record_change(tag, p != nullptr); }
    };

    kids_recorder kids_cb_{this};
    sel_recorder  sel_cb_{this};
    lc_recorder   lc_will_app_{this, "tab_will_appear"};
    lc_recorder   lc_did_app_{this,  "tab_did_appear"};
    lc_recorder   lc_will_dis_{this, "tab_will_disappear"};
    lc_recorder   lc_did_dis_{this,  "tab_did_disappear"};

    signal_slot<const std::vector<basic_page*>&> slot_kids_{};
    signal_slot<const int&>                slot_sel_{};
    signal_slot<basic_page*>                     slot_will_app_{};
    signal_slot<basic_page*>                     slot_did_app_{};
    signal_slot<basic_page*>                     slot_will_dis_{};
    signal_slot<basic_page*>                     slot_did_dis_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_TABBED_PAGE_HANDLER_HPP
