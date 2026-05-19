// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ActivityIndicator.md
//
// `mpapp::activity_indicator` — indeterminate progress spinner. Visible
// + animating when `is_running` is true; hidden otherwise. The `color`
// observable carries a `brush_ref` symbolic tint (real handlers parse
// it into the platform-native color type).

#ifndef MPAPP_ACTIVITY_INDICATOR_HPP
#define MPAPP_ACTIVITY_INDICATOR_HPP

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class activity_indicator_handler;

class activity_indicator : public view {
public:
    activity_indicator() = default;

    Observable<bool>       is_running{false};
    Observable<brush_ref>  color{};

    activity_indicator_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const activity_indicator_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                                 has_handler() const noexcept { return handler_ != nullptr; }
    void                                                 set_handler(activity_indicator_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    activity_indicator_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_ACTIVITY_INDICATOR_HPP
