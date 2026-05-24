// SPDX-License-Identifier: Apache-2.0
// Android basic_tabbed_page handler. Uses a vertical LinearLayout with:
//   - row 0: a horizontal LinearLayout acting as the tab strip
//             (TextView per tab — clickable)
//   - row 1: a FrameLayout content host showing the selected tab's
//             basic_page native.

#ifndef MPAPP_HANDLERS_ANDROID_TABBED_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_TABBED_PAGE_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_tabbed_page.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class tabbed_page_handler<platform::android> {
public:
    tabbed_page_handler();
    ~tabbed_page_handler();

    tabbed_page_handler(const tabbed_page_handler&)            = delete;
    tabbed_page_handler& operator=(const tabbed_page_handler&) = delete;
    tabbed_page_handler(tabbed_page_handler&&)                 = delete;
    tabbed_page_handler& operator=(tabbed_page_handler&&)      = delete;

    void map_children(basic_tabbed_page& tp);
    void map_selected_index(basic_tabbed_page& tp);

    jobject native() const noexcept { return native_; }

private:
    void rebuild_children(const std::vector<basic_page*>& kids);
    void apply_selection(int idx);

    struct children_cb_t {
        tabbed_page_handler<platform::android>* self;
        void operator()(const std::vector<basic_page*>& v) const { self->rebuild_children(v); }
    };
    struct selection_cb_t {
        tabbed_page_handler<platform::android>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };

    jobject native_       = nullptr;  // LinearLayout vertical
    jobject tab_strip_    = nullptr;  // LinearLayout horizontal
    jobject content_host_ = nullptr;  // FrameLayout

    std::vector<basic_page*>   current_kids_{};
    // Tab-strip TextView refs kept around so apply_selection can restyle
    // the active vs inactive tabs. Sized to match current_kids_.
    std::vector<jobject> tab_views_{};

    basic_tabbed_page* bound_ = nullptr;

    children_cb_t  children_cb_{this};
    selection_cb_t selection_cb_{this};
    signal_slot<const std::vector<basic_page*>&> children_slot_{};
    signal_slot<const int&>                selection_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_TABBED_PAGE_HANDLER_HPP
