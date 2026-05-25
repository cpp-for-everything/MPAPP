// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_tabbed_page handler. Uses GtkNotebook for the tab UI.

#ifndef MPAPP_HANDLERS_LINUX_TABBED_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_TABBED_PAGE_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_tabbed_page.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class tabbed_page_handler<platform::linux_> {
public:
    tabbed_page_handler();
    ~tabbed_page_handler();

    tabbed_page_handler(const tabbed_page_handler&)            = delete;
    tabbed_page_handler& operator=(const tabbed_page_handler&) = delete;
    tabbed_page_handler(tabbed_page_handler&&)                 = delete;
    tabbed_page_handler& operator=(tabbed_page_handler&&)      = delete;

    void map_children(basic_tabbed_page& tp);
    void map_selected_index(basic_tabbed_page& tp);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_tabbed_page& x);


private:
    void rebuild_children(const std::vector<basic_page*>& kids);
    void apply_selection(int idx);

    struct children_cb_t {
        tabbed_page_handler<platform::linux_>* self;
        void operator()(const std::vector<basic_page*>& v) const { self->rebuild_children(v); }
    };
    struct selection_cb_t {
        tabbed_page_handler<platform::linux_>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };

    void* native_ = nullptr;  // GtkNotebook*

    basic_tabbed_page* bound_ = nullptr;

    children_cb_t  children_cb_{this};
    selection_cb_t selection_cb_{this};
    signal_slot<const std::vector<basic_page*>&> children_slot_{};
    signal_slot<const int&>                selection_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_TABBED_PAGE_HANDLER_HPP
