// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_box_view handler — wraps `mux::Controls::Border`
// because it exposes both `Background` (the fill) and `CornerRadius`.

#ifndef MPAPP_HANDLERS_WINDOWS_BOX_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_BOX_VIEW_HANDLER_HPP

#include "../../internal/basic_box_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp::internal {

template <>
class box_view_handler<platform::windows> {
public:
    box_view_handler();
    ~box_view_handler();

    box_view_handler(const box_view_handler&)            = delete;
    box_view_handler& operator=(const box_view_handler&) = delete;
    box_view_handler(box_view_handler&&)                 = delete;
    box_view_handler& operator=(box_view_handler&&)      = delete;

    void map_fill(basic_box_view& b);
    void map_corners(basic_box_view& b);

    winrt::Microsoft::UI::Xaml::Controls::Border&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Border& native() const noexcept { return native_; }

private:
    void apply_fill(const color& c);
    void apply_corners(const corner_radius& r);

    struct fill_cb_t {
        box_view_handler<platform::windows>* self = nullptr;
        void operator()(const color& c) const { self->apply_fill(c); }
    };
    struct corners_cb_t {
        box_view_handler<platform::windows>* self = nullptr;
        void operator()(const corner_radius& r) const { self->apply_corners(r); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Border native_{nullptr};
    basic_box_view*                                    bound_ = nullptr;

    fill_cb_t                            fill_cb_{this};
    corners_cb_t                         corners_cb_{this};
    signal_slot<const color&>            fill_slot_{};
    signal_slot<const corner_radius&>    corners_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_BOX_VIEW_HANDLER_HPP
