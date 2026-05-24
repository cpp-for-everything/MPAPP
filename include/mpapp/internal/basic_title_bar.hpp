// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TitleBar.md
//
// `mpapp::title_bar` — custom-content replacement for the OS window title
// bar (Windows + Linux desktops). Exposes two text slots, `title` and
// `subtitle`, that flow through to the native title-bar control on each
// platform. The full MAUI surface (icon / leading_content / content /
// trailing_content / foreground_color / passthrough_elements) lands in
// later batches; this is the M-04b text-only baseline.

#ifndef MPAPP_INTERNAL_BASIC_TITLE_BAR_HPP
#define MPAPP_INTERNAL_BASIC_TITLE_BAR_HPP

#include <string>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class title_bar_handler;

class basic_title_bar : public view {
public:
    basic_title_bar() = default;

    basic_title_bar(const basic_title_bar&)            = delete;
    basic_title_bar& operator=(const basic_title_bar&) = delete;
    basic_title_bar(basic_title_bar&&)                 = delete;
    basic_title_bar& operator=(basic_title_bar&&)      = delete;

    Observable<std::string>  title{};
    Observable<std::string>  subtitle{};

    title_bar_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const title_bar_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                        has_handler() const noexcept { return handler_ != nullptr; }
    void                                        set_handler(title_bar_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    title_bar_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_TITLE_BAR_HPP
