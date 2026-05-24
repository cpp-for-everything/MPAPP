// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Frame.md
//
// `mpapp::frame` — legacy single-child decorator with a colored border,
// corner radius, and optional drop shadow.
//
// **Deprecated**: MAUI .NET 9 deprecates `Frame`; MPAPP mirrors the
// deprecation. New code should prefer [[mpapp::border]] (`<border.hpp>`).
// The control is ported for one-to-one XAML compatibility and migration
// from Xamarin.Forms / early MAUI codebases.

#ifndef MPAPP_FRAME_HPP
#define MPAPP_FRAME_HPP

#include <memory>

#include "box_view.hpp"   // for `color`
#include "layout.hpp"     // for `thickness`
#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform = platform::current>
class frame_handler;

// `[[deprecated]]` attribute is applied so any user code touching the
// type gets a compiler diagnostic suggesting the replacement. This
// matches MAUI's `[Obsolete("Use Border instead.")]` on the C# control.
class [[deprecated("mpapp::frame is deprecated; use mpapp::border instead.")]]
frame : public view {
public:
    frame() = default;

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

} // namespace mpapp

#endif // MPAPP_FRAME_HPP
