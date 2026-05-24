// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — AppKit basic_stack_layout handler implementation.

#include "mpapp/handlers/macos/stack_layout_handler.hpp"

#if defined(__APPLE__) && !TARGET_OS_IPHONE

#import <AppKit/AppKit.h>

#include "mpapp/internal/basic_button.hpp"
#include "mpapp/handlers/macos/button_handler.hpp"
#include "mpapp/handlers/macos/label_handler.hpp"
#include "mpapp/internal/basic_label.hpp"

namespace mpapp::internal {

namespace {

NSLayoutAttribute to_native_align_x(h_align a) {
    switch (a) {
        case h_align::start:   return NSLayoutAttributeLeading;
        case h_align::center:  return NSLayoutAttributeCenterX;
        case h_align::end:     return NSLayoutAttributeTrailing;
        case h_align::stretch: return NSLayoutAttributeWidth;
    }
    return NSLayoutAttributeWidth;
}

NSLayoutAttribute to_native_align_y(v_align a) {
    switch (a) {
        case v_align::start:   return NSLayoutAttributeTop;
        case v_align::center:  return NSLayoutAttributeCenterY;
        case v_align::end:     return NSLayoutAttributeBottom;
        case v_align::stretch: return NSLayoutAttributeHeight;
    }
    return NSLayoutAttributeHeight;
}

} // namespace

stack_layout_handler<platform::macos>::stack_layout_handler() {
    @autoreleasepool {
        NSStackView* sv = [[NSStackView alloc] init];
        sv.orientation = NSUserInterfaceLayoutOrientationVertical;
        sv.spacing     = 0;
        native_        = (__bridge_retained void*)sv;
    }
}

stack_layout_handler<platform::macos>::~stack_layout_handler() {
    if (native_) {
        NSStackView* sv = (__bridge_transfer NSStackView*)native_;
        (void)sv;
        native_ = nullptr;
    }
}

void stack_layout_handler<platform::macos>::apply_orientation(orientation o) {
    if (!native_) return;
    NSStackView* sv = (__bridge NSStackView*)native_;
    sv.orientation = (o == orientation::horizontal)
                       ? NSUserInterfaceLayoutOrientationHorizontal
                       : NSUserInterfaceLayoutOrientationVertical;
}

void stack_layout_handler<platform::macos>::apply_spacing(double s) {
    if (!native_) return;
    NSStackView* sv = (__bridge NSStackView*)native_;
    sv.spacing = s;
}

void stack_layout_handler<platform::macos>::apply_padding(thickness t) {
    if (!native_) return;
    NSStackView* sv = (__bridge NSStackView*)native_;
    sv.edgeInsets = NSEdgeInsetsMake(t.top, t.left, t.bottom, t.right);
}

void stack_layout_handler<platform::macos>::apply_horizontal_alignment(h_align a) {
    if (!native_) return;
    NSStackView* sv = (__bridge NSStackView*)native_;
    sv.alignment = to_native_align_x(a);
}

void stack_layout_handler<platform::macos>::apply_vertical_alignment(v_align a) {
    if (!native_) return;
    NSStackView* sv = (__bridge NSStackView*)native_;
    sv.alignment = to_native_align_y(a);
}

void stack_layout_handler<platform::macos>::bind(basic_stack_layout& s) {
    bound_ = &s;

    apply_orientation(s.stack_orientation.get());
    s.stack_orientation.changed.subscribe(orient_slot_, orient_cb_);

    apply_spacing(s.spacing.get());
    s.spacing.changed.subscribe(spacing_slot_, spacing_cb_);

    apply_padding(s.padding.get());
    s.padding.changed.subscribe(padding_slot_, padding_cb_);

    apply_horizontal_alignment(s.horizontal_alignment.get());
    s.horizontal_alignment.changed.subscribe(h_align_slot_, h_align_cb_);

    apply_vertical_alignment(s.vertical_alignment.get());
    s.vertical_alignment.changed.subscribe(v_align_slot_, v_align_cb_);

    for (std::size_t i = 0; i < s.child_count(); ++i) {
        if (view* child = s.child_at(i); child != nullptr) {
            add_child(*child);
        }
    }
}

void stack_layout_handler<platform::macos>::add_child(view& child) {
    if (!native_) return;
    NSStackView* sv = (__bridge NSStackView*)native_;
    NSView* native_child = nil;
    if (auto* b = dynamic_cast<basic_button*>(&child); b && b->has_handler()) {
        native_child = (__bridge NSView*)b->handler().native();
    } else if (auto* l = dynamic_cast<basic_label*>(&child); l && l->has_handler()) {
        native_child = (__bridge NSView*)l->handler().native();
    } else if (auto* sl = dynamic_cast<basic_stack_layout*>(&child); sl && sl->has_handler()) {
        native_child = (__bridge NSView*)sl->handler().native();
    }
    if (native_child) {
        [sv addArrangedSubview:native_child];
    }
}

} // namespace mpapp::internal
#endif // __APPLE__ && !TARGET_OS_IPHONE
