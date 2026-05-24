// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ShapeView.md
//
// `mpapp::shape_view` — 2D vector primitive (rectangle / ellipse / line
// / polygon / path). Mock surface tracks shape kind + path data + paint
// observables. Real rendering needs a graphics backend (Skia, Cairo,
// or platform-native) — gated on a 2D graphics ADR (TBD).

#ifndef MPAPP_INTERNAL_BASIC_SHAPE_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_SHAPE_VIEW_HPP

#include <cstdint>
#include <string>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp {

enum class shape_kind : std::uint8_t {
    rectangle = 0,
    ellipse   = 1,
    line      = 2,
    polygon   = 3,
    path      = 4,
};

} // namespace mpapp

namespace mpapp::internal {

template <class Platform = platform::current>
class shape_view_handler;

class basic_shape_view : public view {
public:
    basic_shape_view() = default;
    ~basic_shape_view() override = default;

    basic_shape_view(const basic_shape_view&)            = delete;
    basic_shape_view& operator=(const basic_shape_view&) = delete;
    basic_shape_view(basic_shape_view&&)                 = delete;
    basic_shape_view& operator=(basic_shape_view&&)      = delete;

    // ----- Surface ------------------------------------------------------

    Observable<shape_kind>   kind{shape_kind::rectangle};
    Observable<std::string>  data{""};                  // SVG-path-style "M0 0 L10 10 ..."
    Observable<std::string>  fill{""};                  // brush ref (color/gradient name)
    Observable<std::string>  stroke{""};                // brush ref
    Observable<double>       stroke_thickness{1.0};
    Observable<double>       opacity{1.0};

    // ----- Handler ------------------------------------------------------

    shape_view_handler<platform::current>&       sv_handler() noexcept       { return *sv_handler_; }
    const shape_view_handler<platform::current>& sv_handler() const noexcept { return *sv_handler_; }
    bool                                         has_sv_handler() const noexcept { return sv_handler_ != nullptr; }
    void                                         set_sv_handler(shape_view_handler<platform::current>& h) noexcept { sv_handler_ = &h; }

private:
    shape_view_handler<platform::current>* sv_handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_SHAPE_VIEW_HPP
