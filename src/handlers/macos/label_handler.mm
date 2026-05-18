// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — AppKit label handler implementation.

#include "mpapp/handlers/macos/label_handler.hpp"

#if defined(__APPLE__) && !TARGET_OS_IPHONE

#import <AppKit/AppKit.h>

namespace mpapp {

label_handler<platform::macos>::label_handler() {
    @autoreleasepool {
        NSTextField* lbl = [[NSTextField alloc] init];
        [lbl setBezeled:NO];
        [lbl setDrawsBackground:NO];
        [lbl setEditable:NO];
        [lbl setSelectable:NO];
        native_ = (__bridge_retained void*)lbl;
    }
}

label_handler<platform::macos>::~label_handler() {
    if (native_) {
        NSTextField* lbl = (__bridge_transfer NSTextField*)native_;
        (void)lbl;
        native_ = nullptr;
    }
}

void label_handler<platform::macos>::apply_text(const std::string& text) {
    if (!native_) return;
    NSTextField* lbl = (__bridge NSTextField*)native_;
    [lbl setStringValue:[NSString stringWithUTF8String:text.c_str()]];
}

void label_handler<platform::macos>::map_text(label& l) {
    apply_text(l.text.get());
    l.text.changed.subscribe(text_slot_, text_cb_);
}

} // namespace mpapp

#endif // __APPLE__ && !TARGET_OS_IPHONE
