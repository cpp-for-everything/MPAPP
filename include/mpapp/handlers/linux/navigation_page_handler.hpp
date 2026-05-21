// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 navigation_page handler.
//
// Wraps a vertical `GtkBox` with:
//   - top child: a horizontal GtkBox bar (back button + GtkLabel title)
//   - second child: a GtkBox content host swapped on each page_did_appear
//
// Same pattern as content_page_handler<linux_> but driven by the
// page_stack engine instead of a single content Observable.

#ifndef MPAPP_HANDLERS_LINUX_NAVIGATION_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_NAVIGATION_PAGE_HANDLER_HPP

#include <cstddef>
#include <string>

#include "../../navigation_page.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class navigation_page_handler<platform::linux_> {
public:
    navigation_page_handler();
    ~navigation_page_handler();

    navigation_page_handler(const navigation_page_handler&)            = delete;
    navigation_page_handler& operator=(const navigation_page_handler&) = delete;
    navigation_page_handler(navigation_page_handler&&)                 = delete;
    navigation_page_handler& operator=(navigation_page_handler&&)      = delete;

    void map_stack(navigation_page& np);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_top(view* new_top);
    void apply_title(const std::string& v);
    void apply_back_visibility(std::size_t depth);

    struct did_appear_cb {
        navigation_page_handler<platform::linux_>* self;
        void operator()(view* v) const { self->apply_top(v); }
    };
    struct depth_cb {
        navigation_page_handler<platform::linux_>* self;
        void operator()(std::size_t d) const { self->apply_back_visibility(d); }
    };

    void* native_        = nullptr;  // GtkBox* (vertical)
    void* bar_           = nullptr;  // GtkBox* (horizontal)
    void* back_button_   = nullptr;  // GtkButton*
    void* title_label_   = nullptr;  // GtkLabel*
    void* content_host_  = nullptr;  // GtkBox* (single-child wrapper)
    void* current_child_ = nullptr;  // GtkWidget* current top's native

    navigation_page* bound_ = nullptr;

    did_appear_cb                   did_appear_cb_{this};
    depth_cb                        depth_cb_{this};
    signal_slot<view*>              did_appear_slot_{};
    signal_slot<const std::size_t&> depth_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_NAVIGATION_PAGE_HANDLER_HPP
