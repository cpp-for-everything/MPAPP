// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/FlyoutPage.md
//                  vault/20_ADRs/ADR-0014-page-navigation-stack.md
//
// `mpapp::flyout_page` — page-level master/detail container. Has two
// child Page slots (`flyout` and `detail`) and an `is_presented` toggle
// for the flyout pane. Distinct from `flyout_view` (wave-2, view-level)
// which has the same shape but is intended to live inside a page.

#ifndef MPAPP_FLYOUT_PAGE_HPP
#define MPAPP_FLYOUT_PAGE_HPP

#include <cstdint>

#include "observable.hpp"
#include "page.hpp"
#include "platform.hpp"
#include "signal.hpp"

namespace mpapp {

enum class flyout_layout_behavior : std::uint8_t {
    default_           = 0,
    popover            = 1,
    split              = 2,
    split_on_landscape = 3,
    split_on_portrait  = 4,
};

template <class Platform>
class flyout_page_handler;

class flyout_page : public page {
public:
    flyout_page() = default;
    ~flyout_page() override = default;

    flyout_page(const flyout_page&)            = delete;
    flyout_page& operator=(const flyout_page&) = delete;
    flyout_page(flyout_page&&)                 = delete;
    flyout_page& operator=(flyout_page&&)      = delete;

    // ----- Slots --------------------------------------------------------

    Observable<page*>                  flyout{nullptr};
    Observable<page*>                  detail{nullptr};
    Observable<bool>                   is_presented{false};
    Observable<flyout_layout_behavior> layout_behavior{flyout_layout_behavior::default_};

    // ----- Lifecycle signals --------------------------------------------

    signal<bool> presented_changed{};   // emits new value after is_presented flip

    // ----- Mutators -----------------------------------------------------

    void present()    { set_presented(true);  }
    void dismiss()    { set_presented(false); }
    void toggle()     { set_presented(!is_presented.get()); }

    // ----- Handler ------------------------------------------------------

    flyout_page_handler<platform::current>&       fp_handler() noexcept       { return *fp_handler_; }
    const flyout_page_handler<platform::current>& fp_handler() const noexcept { return *fp_handler_; }
    bool                                          has_fp_handler() const noexcept { return fp_handler_ != nullptr; }
    void                                          set_fp_handler(flyout_page_handler<platform::current>& h) noexcept { fp_handler_ = &h; }

private:
    void set_presented(bool v) {
        if (is_presented.get() == v) return;
        is_presented.set(v);
        presented_changed.emit(v);
    }

    flyout_page_handler<platform::current>* fp_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_FLYOUT_PAGE_HPP
