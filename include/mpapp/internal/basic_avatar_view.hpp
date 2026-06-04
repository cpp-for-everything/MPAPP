// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/AvatarView.md
//
// `mpapp::avatar_view` — CommunityToolkit AvatarView surface.
// Displays either a text initials badge or an image; shaped as a
// circle (default) or square with configurable corner radius.
// Mock surface (P2). Mirrors .NET MAUI CommunityToolkit AvatarView.

#ifndef MPAPP_INTERNAL_BASIC_AVATAR_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_AVATAR_VIEW_HPP

#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_AVATAR_VIEW_HAS_STD_FORMAT 1
#endif

#include <cstdint>
#include <string>
#include <string_view>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp {

enum class avatar_shape : std::uint8_t {
    circle = 0,
    square = 1,
};

constexpr std::string_view to_string(avatar_shape s) noexcept {
    switch (s) {
        case avatar_shape::circle: return "circle";
        case avatar_shape::square: return "square";
    }
    return "?";
}

} // namespace mpapp

namespace mpapp::internal {

template <class Platform = platform::current>
class avatar_view_handler;

class basic_avatar_view : public view {
public:
    basic_avatar_view() = default;

    Observable<std::string> initials{""};
    Observable<std::string> image_source{""};
    Observable<double>      corner_radius{0.0};
    Observable<brush_ref>   background{};
    Observable<brush_ref>   text_color{};
    Observable<avatar_shape> shape{avatar_shape::circle};

    avatar_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const avatar_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                          has_handler() const noexcept { return handler_ != nullptr; }
    void                                          set_handler(avatar_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    avatar_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#ifdef MPAPP_AVATAR_VIEW_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::avatar_shape> : std::formatter<std::string_view> {
    auto format(mpapp::avatar_shape s, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(s), ctx);
    }
};

#endif // MPAPP_AVATAR_VIEW_HAS_STD_FORMAT

#endif // MPAPP_INTERNAL_BASIC_AVATAR_VIEW_HPP
