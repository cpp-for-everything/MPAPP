// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit basic_stack_layout handler implementation.

#include "mpapp/handlers/ios/stack_layout_handler.hpp"

#if defined(__APPLE__) && TARGET_OS_IPHONE

#import <UIKit/UIKit.h>

#include "mpapp/internal/basic_button.hpp"
#include "mpapp/handlers/ios/button_handler.hpp"
#include "mpapp/handlers/ios/label_handler.hpp"
#include "mpapp/internal/basic_label.hpp"

namespace mpapp::internal {

namespace {

UIStackViewAlignment to_native_align_h(h_align a) {
    switch (a) {
        case h_align::start:   return UIStackViewAlignmentLeading;
        case h_align::center:  return UIStackViewAlignmentCenter;
        case h_align::end:     return UIStackViewAlignmentTrailing;
        case h_align::stretch: return UIStackViewAlignmentFill;
    }
    return UIStackViewAlignmentFill;
}

UIStackViewAlignment to_native_align_v(v_align a) {
    switch (a) {
        case v_align::start:   return UIStackViewAlignmentTop;
        case v_align::center:  return UIStackViewAlignmentCenter;
        case v_align::end:     return UIStackViewAlignmentBottom;
        case v_align::stretch: return UIStackViewAlignmentFill;
    }
    return UIStackViewAlignmentFill;
}

} // namespace

stack_layout_handler<platform::ios>::stack_layout_handler() {
    @autoreleasepool {
        UIStackView* sv = [[UIStackView alloc] init];
        sv.axis    = UILayoutConstraintAxisVertical;
        sv.spacing = 0;
        native_    = (__bridge_retained void*)sv;
    }
}

stack_layout_handler<platform::ios>::~stack_layout_handler() {
    if (native_) {
        UIStackView* sv = (__bridge_transfer UIStackView*)native_;
        (void)sv;
        native_ = nullptr;
    }
}

void stack_layout_handler<platform::ios>::apply_orientation(orientation o) {
    if (!native_) return;
    UIStackView* sv = (__bridge UIStackView*)native_;
    sv.axis = (o == orientation::horizontal) ? UILayoutConstraintAxisHorizontal
                                             : UILayoutConstraintAxisVertical;
}

void stack_layout_handler<platform::ios>::apply_spacing(double s) {
    if (!native_) return;
    UIStackView* sv = (__bridge UIStackView*)native_;
    sv.spacing = s;
}

void stack_layout_handler<platform::ios>::apply_padding(thickness t) {
    if (!native_) return;
    UIStackView* sv = (__bridge UIStackView*)native_;
    sv.layoutMargins = UIEdgeInsetsMake(t.top, t.left, t.bottom, t.right);
    sv.layoutMarginsRelativeArrangement = YES;
}

void stack_layout_handler<platform::ios>::apply_horizontal_alignment(h_align a) {
    if (!native_) return;
    UIStackView* sv = (__bridge UIStackView*)native_;
    if (sv.axis == UILayoutConstraintAxisVertical) {
        sv.alignment = to_native_align_h(a);
    }
}

void stack_layout_handler<platform::ios>::apply_vertical_alignment(v_align a) {
    if (!native_) return;
    UIStackView* sv = (__bridge UIStackView*)native_;
    if (sv.axis == UILayoutConstraintAxisHorizontal) {
        sv.alignment = to_native_align_v(a);
    }
}

void stack_layout_handler<platform::ios>::bind(basic_stack_layout& s) {
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

void stack_layout_handler<platform::ios>::add_child(view& child) {
    if (!native_) return;
    UIStackView* sv = (__bridge UIStackView*)native_;
    UIView* native_child = nil;
    if (auto* b = dynamic_cast<basic_button*>(&child); b && b->has_handler()) {
        native_child = (__bridge UIView*)b->handler().native();
    } else if (auto* l = dynamic_cast<basic_label*>(&child); l && l->has_handler()) {
        native_child = (__bridge UIView*)l->handler().native();
    } else if (auto* sl = dynamic_cast<basic_stack_layout*>(&child); sl && sl->has_handler()) {
        native_child = (__bridge UIView*)sl->handler().native();
    }
    if (native_child) {
        [sv addArrangedSubview:native_child];
    }
}

} // namespace mpapp::internal
#endif // __APPLE__ && TARGET_OS_IPHONE
