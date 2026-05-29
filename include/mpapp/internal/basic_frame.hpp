// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Frame.md
//
// `mpapp::internal::basic_frame` — platform-agnostic surface for the
// legacy single-child decorator. The `[[deprecated]]` attribute is
// kept on the user-facing `mpapp::frame` wrapper only — the surface
// itself is a normal internal class so framework code that needs to
// inherit / hold it (the wrapper, the handler) does not trip the
// deprecation diagnostic.

#ifndef MPAPP_INTERNAL_BASIC_FRAME_HPP
#define MPAPP_INTERNAL_BASIC_FRAME_HPP

#include <memory>

#include "basic_box_view.hpp"   // for `color`
#include "../layout.hpp"     // for `thickness`
#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class frame_handler;

class basic_frame : public view {
public:
    basic_frame() = default;

    Observable<std::shared_ptr<view>>   content{};
    Observable<color>                   border_color{};
    Observable<bool>                    has_shadow{true};
    Observable<float>                   corner_radius{-1.0f};   // -1 = platform default
    Observable<thickness>               padding{thickness{20.0}};

    frame_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const frame_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                    has_handler() const noexcept { return handler_ != nullptr; }
    void                                    set_handler(frame_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    frame_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal

#endif // MPAPP_INTERNAL_BASIC_FRAME_HPP
