// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit label handler implementation.

#include "mpapp/handlers/ios/label_handler.hpp"

#if defined(__APPLE__) && TARGET_OS_IPHONE

#import <UIKit/UIKit.h>

namespace mpapp {

label_handler<platform::ios>::label_handler() {
    @autoreleasepool {
        UILabel* lbl = [[UILabel alloc] init];
        native_ = (__bridge_retained void*)lbl;
    }
}

label_handler<platform::ios>::~label_handler() {
    if (native_) {
        UILabel* lbl = (__bridge_transfer UILabel*)native_;
        (void)lbl;
        native_ = nullptr;
    }
}

void label_handler<platform::ios>::apply_text(const std::string& text) {
    if (!native_) return;
    UILabel* lbl = (__bridge UILabel*)native_;
    lbl.text = [NSString stringWithUTF8String:text.c_str()];
}

void label_handler<platform::ios>::map_text(label& l) {
    apply_text(l.text.get());
    l.text.changed.subscribe(text_slot_, text_cb_);
}

} // namespace mpapp

#endif // __APPLE__ && TARGET_OS_IPHONE
