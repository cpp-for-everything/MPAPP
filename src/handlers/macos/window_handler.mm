// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — AppKit basic_window handler implementation.

#include "mpapp/handlers/macos/window_handler.hpp"

#if defined(__APPLE__) && !TARGET_OS_IPHONE

#import <AppKit/AppKit.h>

#include "mpapp/internal/basic_button.hpp"
#include "mpapp/handlers/macos/button_handler.hpp"
#include "mpapp/handlers/macos/label_handler.hpp"
#include "mpapp/handlers/macos/stack_layout_handler.hpp"
#include "mpapp/internal/basic_label.hpp"
#include "mpapp/internal/basic_stack_layout.hpp"

@interface MppWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) mpapp::internal::basic_window* boundWindow;
@end

@implementation MppWindowDelegate
- (void)windowWillClose:(NSNotification*)notification {
    (void)notification;
    if (_boundWindow) {
        _boundWindow->closed.emit();
    }
}
@end

namespace mpapp::internal {

window_handler<platform::macos>::window_handler() {
    @autoreleasepool {
        NSWindow* win = [[NSWindow alloc]
            initWithContentRect:NSMakeRect(0, 0, 480, 240)
                      styleMask:NSWindowStyleMaskTitled
                              | NSWindowStyleMaskClosable
                              | NSWindowStyleMaskMiniaturizable
                              | NSWindowStyleMaskResizable
                        backing:NSBackingStoreBuffered
                          defer:NO];
        native_ = (__bridge_retained void*)win;
    }
}

window_handler<platform::macos>::~window_handler() {
    if (native_) {
        NSWindow* win = (__bridge_transfer NSWindow*)native_;
        (void)win;
        native_ = nullptr;
    }
}

void window_handler<platform::macos>::apply_title(const std::string& v) {
    if (!native_) return;
    NSWindow* win = (__bridge NSWindow*)native_;
    [win setTitle:[NSString stringWithUTF8String:v.c_str()]];
}

void window_handler<platform::macos>::apply_content(view* v) {
    if (!native_) return;
    NSWindow* win = (__bridge NSWindow*)native_;
    if (v == nullptr) {
        [win setContentView:nil];
        return;
    }
    NSView* content = nil;
    if (auto* sl = dynamic_cast<basic_stack_layout*>(v); sl && sl->has_handler()) {
        content = (__bridge NSView*)sl->handler().native();
    } else if (auto* b = dynamic_cast<basic_button*>(v); b && b->has_handler()) {
        content = (__bridge NSView*)b->handler().native();
    } else if (auto* l = dynamic_cast<basic_label*>(v); l && l->has_handler()) {
        content = (__bridge NSView*)l->handler().native();
    }
    if (content) {
        [win setContentView:content];
    }
}

void window_handler<platform::macos>::apply_width_or_height() {
    if (!native_ || !bound_) return;
    const int w = bound_->width.get();
    const int h = bound_->height.get();
    if (w <= 0 || h <= 0) return;
    NSWindow* win = (__bridge NSWindow*)native_;
    NSRect frame = [win frame];
    frame.size.width  = w;
    frame.size.height = h;
    [win setFrame:frame display:YES animate:NO];
}

void window_handler<platform::macos>::apply_is_visible(bool v) {
    if (!native_) return;
    NSWindow* win = (__bridge NSWindow*)native_;
    if (v) {
        [win makeKeyAndOrderFront:nil];
    } else {
        [win orderOut:nil];
    }
}

void window_handler<platform::macos>::bind(basic_window& w) {
    bound_ = &w;

    apply_title(w.title.get());
    w.title.changed.subscribe(title_slot_, title_cb_);

    apply_content(w.content.get());
    w.content.changed.subscribe(content_slot_, content_cb_);

    apply_width_or_height();
    w.width.changed.subscribe(width_slot_, width_cb_);
    w.height.changed.subscribe(height_slot_, height_cb_);

    apply_is_visible(w.is_visible.get());
    w.is_visible.changed.subscribe(visible_slot_, visible_cb_);

    NSWindow* win = (__bridge NSWindow*)native_;
    MppWindowDelegate* delegate = [[MppWindowDelegate alloc] init];
    delegate.boundWindow = &w;
    [win setDelegate:delegate];
    // Retain via associated object so delegate outlives bind().
    objc_setAssociatedObject(win, "mpapp_delegate",
                             delegate, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

} // namespace mpapp::internal
#endif // __APPLE__ && !TARGET_OS_IPHONE
