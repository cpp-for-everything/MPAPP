// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/StackLayout.md
//
// `mpapp::stack_layout` — single-axis layout container. Children are
// arranged top-to-bottom (vertical) or left-to-right (horizontal) with
// uniform spacing and configurable alignment. Mirrors MAUI's
// `StackLayout`, WinUI's `StackPanel`, GTK4's `GtkBox`, AppKit's
// `NSStackView`, UIKit's `UIStackView`.
//
// Inherits the child-list API from `mpapp::layout` — call `add()`,
// `insert()`, `remove()`, `clear()`. The four alignment / orientation /
// spacing properties drive the per-platform native widget via
// `stack_layout_handler<platform::current>`.

#ifndef MPAPP_INTERNAL_BASIC_STACK_LAYOUT_HPP
#define MPAPP_INTERNAL_BASIC_STACK_LAYOUT_HPP

#include "../layout.hpp"
#include "../layout_types.hpp"
#include "../observable.hpp"
#include "../platform.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class stack_layout_handler;

class basic_stack_layout : public layout {
public:
    basic_stack_layout() = default;
    ~basic_stack_layout() override = default;

    basic_stack_layout(const basic_stack_layout&)            = delete;
    basic_stack_layout& operator=(const basic_stack_layout&) = delete;
    basic_stack_layout(basic_stack_layout&&)                 = delete;
    basic_stack_layout& operator=(basic_stack_layout&&)      = delete;

    Observable<orientation> stack_orientation{orientation::vertical};
    Observable<double>      spacing{0.0};
    Observable<h_align>     horizontal_alignment{h_align::stretch};
    Observable<v_align>     vertical_alignment{v_align::stretch};

    stack_layout_handler<platform::current>&       handler() noexcept       { return *stack_handler_; }
    const stack_layout_handler<platform::current>& handler() const noexcept { return *stack_handler_; }
    bool                                           has_handler() const noexcept { return stack_handler_ != nullptr; }
    void                                           set_handler(stack_layout_handler<platform::current>& h) noexcept { stack_handler_ = &h; }

private:
    stack_layout_handler<platform::current>* stack_handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_STACK_LAYOUT_HPP
