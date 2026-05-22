// SPDX-License-Identifier: Apache-2.0
// Android shell handler. Horizontal LinearLayout:
//   - left FrameLayout for flyout_content (visibility toggled)
//   - right LinearLayout vertical: tab strip (LinearLayout horizontal of
//     Buttons) + FrameLayout content host.

#ifndef MPAPP_HANDLERS_ANDROID_SHELL_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SHELL_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../shell.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class shell_handler<platform::android> {
public:
    shell_handler();
    ~shell_handler();

    shell_handler(const shell_handler&)            = delete;
    shell_handler& operator=(const shell_handler&) = delete;
    shell_handler(shell_handler&&)                 = delete;
    shell_handler& operator=(shell_handler&&)      = delete;

    void map_tabs(shell& s);
    void map_current_tab_index(shell& s);
    void map_is_flyout_open(shell& s);
    void map_flyout_content(shell& s);
    void map_current_content(shell& s);

    jobject native() const noexcept { return native_; }

private:
    void rebuild_tab_strip(const std::vector<std::string>& labels);
    void apply_selection(int idx);
    void apply_is_flyout_open(bool v);
    void apply_flyout_content(page* p);
    void apply_current_content(page* p);

    struct tabs_cb_t {
        shell_handler<platform::android>* self;
        void operator()(const std::vector<std::string>& v) const { self->rebuild_tab_strip(v); }
    };
    struct sel_cb_t {
        shell_handler<platform::android>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };
    struct flyout_open_cb_t {
        shell_handler<platform::android>* self;
        void operator()(bool v) const { self->apply_is_flyout_open(v); }
    };
    struct flyout_content_cb_t {
        shell_handler<platform::android>* self;
        void operator()(page* p) const { self->apply_flyout_content(p); }
    };
    struct content_cb_t {
        shell_handler<platform::android>* self;
        void operator()(page* p) const { self->apply_current_content(p); }
    };

    jobject native_       = nullptr;  // LinearLayout horizontal (root)
    jobject flyout_host_  = nullptr;  // FrameLayout (left)
    jobject main_host_    = nullptr;  // LinearLayout vertical (right)
    jobject tab_strip_    = nullptr;  // LinearLayout horizontal
    jobject content_host_ = nullptr;  // FrameLayout

    // Tab Button refs retained for selection restyle (size matches
    // current_tab_index domain).
    std::vector<jobject> tab_buttons_{};

    shell* bound_ = nullptr;

    tabs_cb_t           tabs_cb_{this};
    sel_cb_t            sel_cb_{this};
    flyout_open_cb_t    flyout_open_cb_{this};
    flyout_content_cb_t flyout_content_cb_{this};
    content_cb_t        content_cb_{this};
    signal_slot<const std::vector<std::string>&> tabs_slot_{};
    signal_slot<const int&>                       sel_slot_{};
    signal_slot<const bool&>                      flyout_open_slot_{};
    signal_slot<page* const&>                     flyout_content_slot_{};
    signal_slot<page* const&>                     content_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SHELL_HANDLER_HPP
