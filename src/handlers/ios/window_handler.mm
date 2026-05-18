// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit window handler implementation.

#include "mpapp/handlers/ios/window_handler.hpp"

#if defined(__APPLE__) && TARGET_OS_IPHONE

#import <UIKit/UIKit.h>

#include "mpapp/button.hpp"
#include "mpapp/handlers/ios/button_handler.hpp"
#include "mpapp/handlers/ios/label_handler.hpp"
#include "mpapp/handlers/ios/stack_layout_handler.hpp"
#include "mpapp/label.hpp"
#include "mpapp/stack_layout.hpp"

namespace mpapp {

window_handler<platform::ios>::window_handler() {
    @autoreleasepool {
        UIWindow* win = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
        UIViewController* vc = [[UIViewController alloc] init];
        vc.view.backgroundColor = [UIColor systemBackgroundColor];
        win.rootViewController = vc;
        native_  = (__bridge_retained void*)win;
        root_vc_ = (__bridge_retained void*)vc;
    }
}

window_handler<platform::ios>::~window_handler() {
    if (root_vc_) {
        UIViewController* vc = (__bridge_transfer UIViewController*)root_vc_;
        (void)vc;
        root_vc_ = nullptr;
    }
    if (native_) {
        UIWindow* win = (__bridge_transfer UIWindow*)native_;
        (void)win;
        native_ = nullptr;
    }
}

void window_handler<platform::ios>::apply_title(const std::string& v) {
    if (!root_vc_) return;
    UIViewController* vc = (__bridge UIViewController*)root_vc_;
    vc.title = [NSString stringWithUTF8String:v.c_str()];
}

void window_handler<platform::ios>::apply_content(view* v) {
    if (!root_vc_) return;
    UIViewController* vc = (__bridge UIViewController*)root_vc_;

    // Remove existing subviews.
    for (UIView* sub in [vc.view.subviews copy]) {
        [sub removeFromSuperview];
    }
    if (v == nullptr) return;

    UIView* native_child = nil;
    if (auto* sl = dynamic_cast<stack_layout*>(v); sl && sl->has_handler()) {
        native_child = (__bridge UIView*)sl->handler().native();
    } else if (auto* b = dynamic_cast<button*>(v); b && b->has_handler()) {
        native_child = (__bridge UIView*)b->handler().native();
    } else if (auto* l = dynamic_cast<label*>(v); l && l->has_handler()) {
        native_child = (__bridge UIView*)l->handler().native();
    }
    if (native_child) {
        native_child.translatesAutoresizingMaskIntoConstraints = NO;
        [vc.view addSubview:native_child];
        [NSLayoutConstraint activateConstraints:@[
            [native_child.centerXAnchor constraintEqualToAnchor:vc.view.safeAreaLayoutGuide.centerXAnchor],
            [native_child.centerYAnchor constraintEqualToAnchor:vc.view.safeAreaLayoutGuide.centerYAnchor],
        ]];
    }
}

void window_handler<platform::ios>::apply_is_visible(bool v) {
    if (!native_) return;
    UIWindow* win = (__bridge UIWindow*)native_;
    if (v) {
        [win makeKeyAndVisible];
    } else {
        win.hidden = YES;
    }
}

void window_handler<platform::ios>::bind(window& w) {
    bound_ = &w;

    apply_title(w.title.get());
    w.title.changed.subscribe(title_slot_, title_cb_);

    apply_content(w.content.get());
    w.content.changed.subscribe(content_slot_, content_cb_);

    apply_is_visible(w.is_visible.get());
    w.is_visible.changed.subscribe(visible_slot_, visible_cb_);
}

} // namespace mpapp

#endif // __APPLE__ && TARGET_OS_IPHONE
