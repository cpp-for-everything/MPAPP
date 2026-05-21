// SPDX-License-Identifier: Apache-2.0
// GTK4 flyout_page handler. Uses GtkPaned horizontal as the master-detail
// split; the flyout pane's visibility is toggled by is_presented.

#ifndef MPAPP_HANDLERS_LINUX_FLYOUT_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_FLYOUT_PAGE_HANDLER_HPP

#include "../../flyout_page.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class flyout_page_handler<platform::linux_> {
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

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_flyout(page* p);
    void apply_detail(page* p);
    void apply_is_presented(bool v);

    struct flyout_cb_t {
        flyout_page_handler<platform::linux_>* self;
        void operator()(page* p) const { self->apply_flyout(p); }
    };
    struct detail_cb_t {
        flyout_page_handler<platform::linux_>* self;
        void operator()(page* p) const { self->apply_detail(p); }
    };
    struct presented_cb_t {
        flyout_page_handler<platform::linux_>* self;
        void operator()(bool v) const { self->apply_is_presented(v); }
    };

    void* native_       = nullptr;  // GtkPaned*
    void* flyout_slot_w_ = nullptr; // GtkBox wrapper for flyout child
    void* detail_slot_w_ = nullptr; // GtkBox wrapper for detail child
    void* current_flyout_child_ = nullptr;
    void* current_detail_child_ = nullptr;

    flyout_cb_t    flyout_cb_{this};
    detail_cb_t    detail_cb_{this};
    presented_cb_t presented_cb_{this};
    signal_slot<page* const&> flyout_slot_{};
    signal_slot<page* const&> detail_slot_{};
    signal_slot<const bool&>  presented_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_FLYOUT_PAGE_HANDLER_HPP
