// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/DatePicker.md
//
// `mpapp::date_picker` — calendar date input. Carries a simple POD
// `date_value{ year, month, day }` to avoid dragging in <chrono> at
// the cross-platform layer; real handlers convert to / from their
// native date types.

#ifndef MPAPP_DATE_PICKER_HPP
#define MPAPP_DATE_PICKER_HPP

#include <string>

#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_DATE_PICKER_HAS_STD_FORMAT 1
#endif

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

struct date_value {
    int year  = 1970;
    int month = 1;     // 1..12
    int day   = 1;     // 1..31

    bool operator==(const date_value&) const = default;
};

template <class Platform>
class date_picker_handler;

class date_picker : public view {
public:
    date_picker() = default;

    Observable<date_value>      date{};
    Observable<std::string>     format{"D"};   // platform interpretation; "D" = long date

    date_picker_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const date_picker_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                          has_handler() const noexcept { return handler_ != nullptr; }
    void                                          set_handler(date_picker_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    date_picker_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#ifdef MPAPP_DATE_PICKER_HAS_STD_FORMAT
template <>
struct std::formatter<mpapp::date_value> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::date_value& d, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{:04d}-{:02d}-{:02d}", d.year, d.month, d.day);
    }
};
#endif

#endif // MPAPP_DATE_PICKER_HPP
