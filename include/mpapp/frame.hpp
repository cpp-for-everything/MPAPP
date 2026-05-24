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

#include "internal/basic_frame.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_frame` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/frame_handler.hpp"

namespace mpapp {

// `[[deprecated]]` attribute is applied so any user code touching the
// type gets a compiler diagnostic suggesting the replacement. This
// matches MAUI's `[Obsolete("Use Border instead.")]` on the C# control.
// The surface (`internal::basic_frame`) is NOT marked deprecated so the
// wrapper + handler can inherit / hold it without warnings.
class [[deprecated("mpapp::frame is deprecated; use mpapp::border instead.")]]
frame : public internal::basic_frame {
public:
    frame() {
        set_handler(embedded_handler_);
        embedded_handler_.map_content(*this);
        embedded_handler_.map_border_color(*this);
        embedded_handler_.map_has_shadow(*this);
        embedded_handler_.map_corner_radius(*this);
        embedded_handler_.map_padding(*this);
    }

    frame(const frame&)            = delete;
    frame& operator=(const frame&) = delete;
    frame(frame&&)                 = delete;
    frame& operator=(frame&&)      = delete;

private:
    internal::frame_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::frame_handler<>` (host-current) and
// `mpapp::frame_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using frame_handler = internal::frame_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_FRAME_HPP
