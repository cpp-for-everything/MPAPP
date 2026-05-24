// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit basic_button handler implementation.

#include "mpapp/handlers/ios/button_handler.hpp"

#if defined(__APPLE__) && TARGET_OS_IPHONE

#import <UIKit/UIKit.h>

@interface MppUIButtonTarget : NSObject
@property (nonatomic, assign) mpapp::internal::basic_button* boundButton;
- (void)onTouchUpInside:(id)sender;
@end

@implementation MppUIButtonTarget
- (void)onTouchUpInside:(id)sender {
    (void)sender;
    if (_boundButton) {
        _boundButton->clicked.emit();
    }
}
@end

namespace mpapp::internal {

button_handler<platform::ios>::button_handler() {
    @autoreleasepool {
        UIButton* btn = [UIButton buttonWithType:UIButtonTypeSystem];
        native_ = (__bridge_retained void*)btn;
    }
}

button_handler<platform::ios>::~button_handler() {
    if (target_) {
        MppUIButtonTarget* t = (__bridge_transfer MppUIButtonTarget*)target_;
        (void)t;
        target_ = nullptr;
    }
    if (native_) {
        UIButton* btn = (__bridge_transfer UIButton*)native_;
        (void)btn;
        native_ = nullptr;
    }
}

void button_handler<platform::ios>::apply_text(const std::string& text) {
    if (!native_) return;
    UIButton* btn = (__bridge UIButton*)native_;
    [btn setTitle:[NSString stringWithUTF8String:text.c_str()]
         forState:UIControlStateNormal];
}

void button_handler<platform::ios>::map_text(basic_button& b) {
    apply_text(b.text.get());
    b.text.changed.subscribe(text_slot_, text_cb_);
}

void button_handler<platform::ios>::map_clicked(basic_button& b) {
    if (!native_) return;
    UIButton* btn = (__bridge UIButton*)native_;

    MppUIButtonTarget* tgt = [[MppUIButtonTarget alloc] init];
    tgt.boundButton = &b;
    target_ = (__bridge_retained void*)tgt;

    [btn addTarget:tgt
            action:@selector(onTouchUpInside:)
  forControlEvents:UIControlEventTouchUpInside];
}

} // namespace mpapp::internal

#endif // __APPLE__ && TARGET_OS_IPHONE
