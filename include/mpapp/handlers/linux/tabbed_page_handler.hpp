// SPDX-License-Identifier: Apache-2.0
// GTK4 tabbed_page handler. Uses GtkNotebook for the tab UI.

#ifndef MPAPP_HANDLERS_LINUX_TABBED_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_TABBED_PAGE_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../tabbed_page.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class tabbed_page_handler<platform::linux_> {
public:
    tabbed_page_handler();
    ~tabbed_page_handler();

    tabbed_page_handler(const tabbed_page_handler&)            = delete;
    tabbed_page_handler& operator=(const tabbed_page_handler&) = delete;
    tabbed_page_handler(tabbed_page_handler&&)                 = delete;
    tabbed_page_handler& operator=(tabbed_page_handler&&)      = delete;

    void map_children(tabbed_page& tp);
    void map_selected_index(tabbed_page& tp);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void rebuild_children(const std::vector<page*>& kids);
    void apply_selection(int idx);

    struct children_cb_t {
        tabbed_page_handler<platform::linux_>* self;
        void operator()(const std::vector<page*>& v) const { self->rebuild_children(v); }
    };
    struct selection_cb_t {
        tabbed_page_handler<platform::linux_>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };

    void* native_ = nullptr;  // GtkNotebook*

    tabbed_page* bound_ = nullptr;

    children_cb_t  children_cb_{this};
    selection_cb_t selection_cb_{this};
    signal_slot<const std::vector<page*>&> children_slot_{};
    signal_slot<const int&>                selection_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_TABBED_PAGE_HANDLER_HPP
