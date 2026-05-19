// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ProgressBar.md
//
// `mpapp::progress_bar` — horizontal progress indicator. `progress` is a
// normalized 0..1 value; the real handlers map to platform-native ranges
// (WinUI 3 sets Maximum=1.0 and Value=progress; GTK4 calls
// gtk_progress_bar_set_fraction; Android scales to int 0..1000).

#ifndef MPAPP_PROGRESS_BAR_HPP
#define MPAPP_PROGRESS_BAR_HPP

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class progress_bar_handler;

class progress_bar : public view {
public:
    progress_bar() = default;

    Observable<double>      progress{0.0};        // 0..1
    Observable<brush_ref>   color{};
    Observable<brush_ref>   background_color{};

    progress_bar_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const progress_bar_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                           has_handler() const noexcept { return handler_ != nullptr; }
    void                                           set_handler(progress_bar_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    progress_bar_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_PROGRESS_BAR_HPP
