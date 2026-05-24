// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_tabbed_view handler.
//
// Deliberately avoids the spec's preferred `TabLayout + ViewPager2`
// pair (it requires androidx, which the minimal `android_hello`
// example does not link in) and the framework-level `TabHost` (which
// requires either an inflated layout resource or a TabContentFactory
// implemented via JNI to satisfy `setup()` — both heavyweight for the
// "show one of N labelled pages at a time" contract). The component
// doc explicitly allows falling back to a simpler primitive.
//
// Layout shape:
//
//   LinearLayout (vertical, the host ViewGroup)
//   ├─ LinearLayout (horizontal — the tab strip; one Button per title)
//   └─ FrameLayout  (the basic_page host)
//      ├─ FrameLayout (placeholder for basic_page 0)
//      ├─ FrameLayout (placeholder for basic_page 1)
//      └─ ...
//
// `selected_index` toggles the placeholders' visibility (GONE for the
// inactive pages, VISIBLE for the active one). Real basic_page bodies land
// when the `TabbedPage` basic_page-level wiring arrives.
//
// JNI conventions follow the rest of the Android handlers — every
// helper starts with `if (env->ExceptionCheck()) env->ExceptionClear()`
// and the native object is held as a JNI global ref that the dtor
// releases.

#ifndef MPAPP_HANDLERS_ANDROID_TABBED_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_TABBED_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_tabbed_view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class tabbed_view_handler<platform::android> {
public:
    tabbed_view_handler();
    ~tabbed_view_handler();

    tabbed_view_handler(const tabbed_view_handler&)            = delete;
    tabbed_view_handler& operator=(const tabbed_view_handler&) = delete;
    tabbed_view_handler(tabbed_view_handler&&)                 = delete;
    tabbed_view_handler& operator=(tabbed_view_handler&&)      = delete;

    void map_tab_titles(basic_tabbed_view& t);
    void map_selected_index(basic_tabbed_view& t);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_tab_titles(const std::vector<std::string>& v);
    void apply_selected_index(int v);

    struct tab_titles_cb_t     { tabbed_view_handler<platform::android>* self; void operator()(const std::vector<std::string>& v) const { self->apply_tab_titles(v); } };
    struct selected_index_cb_t { tabbed_view_handler<platform::android>* self; void operator()(int v) const { self->apply_selected_index(v); } };

    // Host vertical LinearLayout global ref (the outer ViewGroup).
    jobject native_           = nullptr;
    // Inner FrameLayout that holds the per-tab content placeholders.
    // Held as a global ref so we can repopulate it on tab_titles
    // changes and toggle child visibility on selected_index changes;
    // released alongside `native_` in the dtor.
    jobject content_frame_    = nullptr;

    bool    suppress_echo_    = false;

    tab_titles_cb_t                              tab_titles_cb_{this};
    selected_index_cb_t                          selected_index_cb_{this};
    signal_slot<std::vector<std::string> const&> tab_titles_slot_{};
    signal_slot<const int&>                      selected_index_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_TABBED_VIEW_HANDLER_HPP
