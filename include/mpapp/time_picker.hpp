// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TimePicker.md
//
// `mpapp::time_picker` — clock time input (hour + minute, 24h).

#ifndef MPAPP_TIME_PICKER_HPP
#define MPAPP_TIME_PICKER_HPP

#include <string>

#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_TIME_PICKER_HAS_STD_FORMAT 1
#endif

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

struct time_value {
    int hour   = 0;   // 0..23
    int minute = 0;   // 0..59

    bool operator==(const time_value&) const = default;
};

template <class Platform>
class time_picker_handler;

class time_picker : public view {
public:
    time_picker() = default;

    Observable<time_value>      time{};
    Observable<std::string>     format{"t"};   // platform interpretation

    time_picker_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const time_picker_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                          has_handler() const noexcept { return handler_ != nullptr; }
    void                                          set_handler(time_picker_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    time_picker_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#ifdef MPAPP_TIME_PICKER_HAS_STD_FORMAT
template <>
struct std::formatter<mpapp::time_value> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::time_value& t, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{:02d}:{:02d}", t.hour, t.minute);
    }
};
#endif

#endif // MPAPP_TIME_PICKER_HPP
