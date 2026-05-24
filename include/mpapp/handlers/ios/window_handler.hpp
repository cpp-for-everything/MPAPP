// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit basic_window handler. Wraps UIWindow.

#ifndef MPAPP_HANDLERS_IOS_WINDOW_HANDLER_HPP
#define MPAPP_HANDLERS_IOS_WINDOW_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_window.hpp"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE

#include <string>

namespace mpapp::internal {

template <>
class window_handler<platform::ios> {
public:
    window_handler();
    ~window_handler();

    window_handler(const window_handler&)            = delete;
    window_handler& operator=(const window_handler&) = delete;
    window_handler(window_handler&&)                 = delete;
    window_handler& operator=(window_handler&&)      = delete;

    void bind(basic_window& w);

    // UIWindow*, retained.
    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_title(const std::string& v);
    void apply_content(view* v);
    void apply_is_visible(bool v);

    struct title_cb_t   { window_handler<platform::ios>* self; void operator()(const std::string& v) const { self->apply_title(v); } };
    struct content_cb_t { window_handler<platform::ios>* self; void operator()(view* v) const { self->apply_content(v); } };
    struct visible_cb_t { window_handler<platform::ios>* self; void operator()(bool v) const { self->apply_is_visible(v); } };

    void*   native_  = nullptr;  // retained UIWindow*
    void*   root_vc_ = nullptr;  // retained UIViewController*
    basic_window* bound_   = nullptr;

    title_cb_t                       title_cb_{this};
    content_cb_t                     content_cb_{this};
    visible_cb_t                     visible_cb_{this};
    signal_slot<const std::string&>  title_slot_{};
    signal_slot<view* const&>        content_slot_{};
    signal_slot<const bool&>         visible_slot_{};
};

} // namespace mpapp::internal
#  endif // TARGET_OS_IPHONE
#endif // __APPLE__
#endif // MPAPP_HANDLERS_IOS_WINDOW_HANDLER_HPP
