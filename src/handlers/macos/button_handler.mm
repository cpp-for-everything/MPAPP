// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — AppKit button handler implementation.

#include "mpapp/handlers/macos/button_handler.hpp"

#if defined(__APPLE__) && !TARGET_OS_IPHONE

#import <AppKit/AppKit.h>

@interface MppButtonTarget : NSObject
@property (nonatomic, assign) mpapp::button* boundButton;
- (void)onClicked:(id)sender;
@end

@implementation MppButtonTarget
- (void)onClicked:(id)sender {
    (void)sender;
    if (_boundButton) {
        _boundButton->clicked.emit();
    }
}
@end

namespace mpapp {

button_handler<platform::macos>::button_handler() {
    @autoreleasepool {
        NSButton* btn = [[NSButton alloc] init];
        [btn setBezelStyle:NSBezelStyleRounded];
        [btn setButtonType:NSButtonTypeMomentaryPushIn];
        native_ = (__bridge_retained void*)btn;
    }
}

button_handler<platform::macos>::~button_handler() {
    if (native_) {
        NSButton* btn = (__bridge_transfer NSButton*)native_;
        (void)btn;
        native_ = nullptr;
    }
    if (target_) {
        MppButtonTarget* t = (__bridge_transfer MppButtonTarget*)target_;
        (void)t;
        target_ = nullptr;
    }
}

void button_handler<platform::macos>::apply_text(const std::string& text) {
    if (!native_) return;
    NSButton* btn = (__bridge NSButton*)native_;
    [btn setTitle:[NSString stringWithUTF8String:text.c_str()]];
}

void button_handler<platform::macos>::map_text(button& b) {
    apply_text(b.text.get());
    b.text.changed.subscribe(text_slot_, text_cb_);
}

void button_handler<platform::macos>::map_clicked(button& b) {
    if (!native_) return;
    NSButton* btn = (__bridge NSButton*)native_;

    MppButtonTarget* tgt = [[MppButtonTarget alloc] init];
    tgt.boundButton = &b;
    target_ = (__bridge_retained void*)tgt;

    [btn setTarget:tgt];
    [btn setAction:@selector(onClicked:)];
}

} // namespace mpapp

#endif // __APPLE__ && !TARGET_OS_IPHONE
