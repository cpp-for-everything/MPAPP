// SPDX-License-Identifier: Apache-2.0
// UIKit basic_carousel_view handler implementation.
//
// BLIND WRITE — compiled + run on a Mac: PENDING (no Apple host/SDK on the
// dev machine). Follows the UIKit seed-handler conventions
// (__bridge_retained void* native_, @autoreleasepool in ctor). No
// per-widget dispatch registry on Apple yet — that lands with the Apple
// handler sweep.

#include "mpapp/handlers/ios/carousel_view_handler.hpp"

#if defined(__APPLE__) && TARGET_OS_IPHONE

#import <UIKit/UIKit.h>

namespace mpapp::internal {

carousel_view_handler<platform::ios>::carousel_view_handler() {
    @autoreleasepool {
        UIView* container = [[UIView alloc] initWithFrame:CGRectZero];
        native_ = (__bridge_retained void*)container;
    }
}

carousel_view_handler<platform::ios>::~carousel_view_handler() {
    if (native_) {
        UIView* container = (__bridge_transfer UIView*)native_;
        (void)container;
        native_ = nullptr;
    }
}

void carousel_view_handler<platform::ios>::rebuild_items(const std::vector<std::string>& v) {
    if (!native_) return;
    @autoreleasepool {
        UIView* container = (__bridge UIView*)native_;
        for (UIView* sub in [[container subviews] copy]) {
            [sub removeFromSuperview];
        }
        for (const auto& s : v) {
            UILabel* lbl = [[UILabel alloc] initWithFrame:CGRectZero];
            [lbl setText:[NSString stringWithUTF8String:s.c_str()]];
            [lbl setTextAlignment:NSTextAlignmentCenter];
            [lbl setHidden:YES];
            [container addSubview:lbl];
        }
    }
    if (bound_) apply_position(bound_->position.get());
}

void carousel_view_handler<platform::ios>::apply_position(int idx) {
    if (!native_) return;
    @autoreleasepool {
        UIView* container = (__bridge UIView*)native_;
        NSArray<UIView*>* subs = [container subviews];
        const NSInteger n = (NSInteger)[subs count];
        for (NSInteger i = 0; i < n; ++i) {
            [[subs objectAtIndex:i] setHidden:(i != (NSInteger)idx)];
        }
    }
}

void carousel_view_handler<platform::ios>::map_items_source(basic_carousel_view& c) {
    bound_ = &c;
    rebuild_items(c.items_source.get());
    c.items_source.changed.subscribe(items_slot_, items_cb_);
}

void carousel_view_handler<platform::ios>::map_position(basic_carousel_view& c) {
    apply_position(c.position.get());
    c.position.changed.subscribe(pos_slot_, pos_cb_);
}

void carousel_view_handler<platform::ios>::map_loop(basic_carousel_view& /*c*/) {
    // Loop/clamp handled in basic_carousel_view::scroll_to.
}

void carousel_view_handler<platform::ios>::map_is_swipe_enabled(basic_carousel_view& /*c*/) {
    // UIPageViewController-based swipe paging is a follow-up (blind).
}

void carousel_view_handler<platform::ios>::map_peek_count(basic_carousel_view& /*c*/) {
    // Single visible page — peek is a v1 no-op.
}

} // namespace mpapp::internal
#endif // __APPLE__ && TARGET_OS_IPHONE
