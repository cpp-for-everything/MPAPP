// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit basic_label handler implementation.

#include "mpapp/handlers/ios/label_handler.hpp"

#if defined(__APPLE__) && TARGET_OS_IPHONE

#import <UIKit/UIKit.h>

namespace mpapp::internal {

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

void label_handler<platform::ios>::map_text(basic_label& l) {
    apply_text(l.text.get());
    l.text.changed.subscribe(text_slot_, text_cb_);
}

void label_handler<platform::ios>::apply_font() {
    if (!native_) return;
    UILabel* lbl = (__bridge UILabel*)native_;
    CGFloat size = font_size_ > 0.0 ? (CGFloat)font_size_ : [UIFont labelFontSize];
    UIFont* font = nil;
    if (!font_family_.empty()) {
        font = [UIFont fontWithName:[NSString stringWithUTF8String:font_family_.c_str()]
                               size:size];
    }
    if (!font) {
        font = font_bold_ ? [UIFont boldSystemFontOfSize:size]
                          : [UIFont systemFontOfSize:size];
    }
    lbl.font = font;
    if (text_color_.a > 0.0) {
        lbl.textColor = [UIColor colorWithRed:text_color_.r
                                        green:text_color_.g
                                         blue:text_color_.b
                                        alpha:text_color_.a];
    }
}

void label_handler<platform::ios>::map_font_size(basic_label& l) {
    font_size_ = l.font_size.get();
    apply_font();
    l.font_size.changed.subscribe(fsize_slot_, fsize_cb_);
}

void label_handler<platform::ios>::map_font_bold(basic_label& l) {
    font_bold_ = l.font_bold.get();
    apply_font();
    l.font_bold.changed.subscribe(fbold_slot_, fbold_cb_);
}

void label_handler<platform::ios>::map_font_family(basic_label& l) {
    font_family_ = l.font_family.get();
    apply_font();
    l.font_family.changed.subscribe(ffamily_slot_, ffamily_cb_);
}

void label_handler<platform::ios>::map_text_color(basic_label& l) {
    text_color_ = l.text_color.get();
    apply_font();
    l.text_color.changed.subscribe(tcolor_slot_, tcolor_cb_);
}

} // namespace mpapp::internal
#endif // __APPLE__ && TARGET_OS_IPHONE
